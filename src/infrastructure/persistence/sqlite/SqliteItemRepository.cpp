#include <stdexcept>
#include <boost/json.hpp>
#include "infrastructure/persistence/sqlite/SqliteItemRepository.hpp"

namespace infrastructure {

static const std::string select = "SELECT items.id, items.name, types.name, items.ingredients, items.recipe_text FROM items JOIN types ON items.type_id = types.id";

static std::vector<std::string> read_ingredients(sqlite3_stmt* stmt, int column) {
    const auto* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, column));
    if (!text) return {};

    std::vector<std::string> result;
    const auto parsed = boost::json::parse(text).as_array();
    result.reserve(parsed.size());
    for (const auto& ingredient : parsed)
        result.emplace_back(ingredient.as_string());
    return result;
}

static std::string ingredients_json(const std::vector<std::string>& ingredients) {
    boost::json::array array;
    for (const auto& ingredient : ingredients)
        array.emplace_back(ingredient);
    return boost::json::serialize(array);
}

SqliteItemRepository::SqliteItemRepository(SqliteConnection& con) : con_(con) {}

std::optional<domain::Item> SqliteItemRepository::get_by_id(const int& id) {
    const std::string sql = select + " WHERE items.id = ?";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    

    std::optional<domain::Item> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = domain::Item{
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            read_ingredients(stmt, 3),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
        };
    }

    sqlite3_finalize(stmt);
    return result;
}

std::vector<domain::Item>   SqliteItemRepository::list() {
    std::vector<domain::Item> list;
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), select.c_str(), -1, &stmt, nullptr);
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        list.push_back(domain::Item{
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            read_ingredients(stmt, 3),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
        });
    }
    
    sqlite3_finalize(stmt);
    return list;
}

std::vector<std::pair<int, std::string>> SqliteItemRepository::list_types() {
    std::vector<std::pair<int, std::string>> types;
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), "SELECT id, name FROM types ORDER BY id", -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        types.emplace_back(sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    }
    sqlite3_finalize(stmt);
    return types;
}

std::vector<std::pair<int, std::string>> SqliteItemRepository::list_daytime_types() {
    std::vector<std::pair<int, std::string>> daytime_types;
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), "SELECT id, name FROM daytime_types ORDER BY id", -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        daytime_types.emplace_back(sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    }
    sqlite3_finalize(stmt);
    return daytime_types;
}

std::pair<int, std::string> SqliteItemRepository::create_type(const std::string& name) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT INTO types(name) VALUES(?)";
    if (sqlite3_prepare_v2(con_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("sqlite prepare failure: " + std::string(sqlite3_errmsg(con_.handle())));
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        const std::string err = sqlite3_errmsg(con_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("sqlite insert failure: " + err);
    }
    sqlite3_finalize(stmt);
    return {static_cast<int>(sqlite3_last_insert_rowid(con_.handle())), name};
}

std::optional<std::pair<int, std::string>> SqliteItemRepository::update_type(const int& id, const std::string& name) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "UPDATE types SET name = ? WHERE id = ?";
    if (sqlite3_prepare_v2(con_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("sqlite prepare failure: " + std::string(sqlite3_errmsg(con_.handle())));
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        const std::string err = sqlite3_errmsg(con_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("sqlite update failure: " + err);
    }
    const bool updated = sqlite3_changes(con_.handle()) > 0;
    sqlite3_finalize(stmt);
    if (!updated) return std::nullopt;
    return std::make_pair(id, name);
}

bool SqliteItemRepository::remove_type(const int& id) {
    auto* db = con_.handle();
    char* error = nullptr;
    if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, &error) != SQLITE_OK) {
        const std::string message = error ? error : sqlite3_errmsg(db);
        sqlite3_free(error);
        throw std::runtime_error("sqlite transaction failure: " + message);
    }

    auto rollback = [&]() { sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr); };
    auto delete_by_type = [&](const char* sql) {
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error("sqlite prepare failure: " + std::string(sqlite3_errmsg(db)));
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            const std::string message = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw std::runtime_error("sqlite delete failure: " + message);
        }
        sqlite3_finalize(stmt);
        return sqlite3_changes(db) > 0;
    };

    try {
        delete_by_type("DELETE FROM items WHERE type_id = ?");
        const bool removed = delete_by_type("DELETE FROM types WHERE id = ?");
        if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, &error) != SQLITE_OK) {
            const std::string message = error ? error : sqlite3_errmsg(db);
            sqlite3_free(error);
            rollback();
            throw std::runtime_error("sqlite commit failure: " + message);
        }
        return removed;
    } catch (...) {
        rollback();
        throw;
    }
}

std::vector<domain::Item>   SqliteItemRepository::list_by_type(const std::string& t) {
    return list_by_filters(std::stoi(t), std::nullopt);
}

std::vector<domain::Item> SqliteItemRepository::list_by_filters(
    const std::optional<int>& type_id,
    const std::optional<int>& daytime_type_id) {
    std::vector<domain::Item> list;

    std::string filtered_select = select;
    if (type_id || daytime_type_id) {
        filtered_select += " WHERE ";
        if (type_id) filtered_select += "items.type_id = ?";
        if (type_id && daytime_type_id) filtered_select += " AND ";
        if (daytime_type_id) filtered_select += "items.daytime_type_id = ?";
    }
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), filtered_select.c_str(), -1, &stmt, nullptr);
    int parameter = 1;
    if (type_id) sqlite3_bind_int(stmt, parameter++, *type_id);
    if (daytime_type_id) sqlite3_bind_int(stmt, parameter, *daytime_type_id);
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        list.push_back(domain::Item{
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            read_ingredients(stmt, 3),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
        });
    }

    sqlite3_finalize(stmt);
    return list;
}

domain::Item                SqliteItemRepository::create(const domain::Item& item) {
    const std::string sql = "INSERT INTO items(name, type_id, ingredients, recipe_text) VALUES(?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, item.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, std::stoi(item.type));
    const auto serialized_ingredients = ingredients_json(item.ingredients);
    sqlite3_bind_text(stmt, 3, serialized_ingredients.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, item.recipe_text.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err_msg = sqlite3_errmsg(con_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("sqlite insert failure: " + err_msg);
    }

    sqlite3_finalize(stmt);

    auto created = get_by_id(static_cast<int>(sqlite3_last_insert_rowid(con_.handle())));
    if (!created)
        throw std::runtime_error("item was inserted but could not be retrieved");
    return *created;
}

std::optional<domain::Item> SqliteItemRepository::update(const int& id, const domain::Item& item) {
    const std::string sql = "UPDATE items SET name = ?, type_id = ?, ingredients = ?, recipe_text = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, item.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, std::stoi(item.type));
    const auto serialized_ingredients = ingredients_json(item.ingredients);
    sqlite3_bind_text(stmt, 3, serialized_ingredients.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, item.recipe_text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, id);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    if (!ok) {
        return std::nullopt;
    }

    return get_by_id(id);
}

bool                        SqliteItemRepository::remove(const int& id) {
    const std::string sql = "DELETE FROM items WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(con_.handle()) > 0;
}

}
