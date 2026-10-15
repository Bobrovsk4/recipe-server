#include <stdexcept>
#include <boost/json.hpp>
#include "infrastructure/persistence/sqlite/SqliteDrinkRepository.hpp"

namespace infrastructure {

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

SqliteDrinkRepository::SqliteDrinkRepository(SqliteConnection& con) : con_(con) {}

std::optional<domain::Drink> SqliteDrinkRepository::get_by_id(const int& id) {
    const std::string sql = "SELECT * FROM drinks WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    

    std::optional<domain::Drink> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = domain::Drink{
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            sqlite3_column_int(stmt, 2),
            read_ingredients(stmt, 3),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
        };
    }

    sqlite3_finalize(stmt);
    return result;
}

std::vector<domain::Drink> SqliteDrinkRepository::list() {
    std::vector<domain::Drink> list;
    sqlite3_stmt* stmt = nullptr;
    std::string select = "SELECT * FROM drinks";
    sqlite3_prepare_v2(con_.handle(), select.c_str(), -1, &stmt, nullptr);
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        list.push_back(domain::Drink{
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            sqlite3_column_int(stmt, 2),
            read_ingredients(stmt, 3),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
        });
    }
    
    sqlite3_finalize(stmt);
    return list;
}

std::vector<std::pair<int, std::string>> SqliteDrinkRepository::list_types() {
    std::vector<std::pair<int, std::string>> types;
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), "SELECT id, name FROM drink_types ORDER BY id", -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        types.emplace_back(sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    }
    sqlite3_finalize(stmt);
    return types;
}

std::vector<domain::Drink> SqliteDrinkRepository::list_by_type(const std::string& t) {
    std::vector<domain::Drink> list;
    std::optional<int> type_id = std::stoi(t);
    std::string request = "SELECT * FROM drinks WHERE type_id = ?";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), request.c_str(), -1, &stmt, nullptr);
    if (type_id) sqlite3_bind_int(stmt, 1, *type_id);

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        list.push_back(domain::Drink{
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            sqlite3_column_int(stmt, 2),
            read_ingredients(stmt, 3),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
        });
    }

    sqlite3_finalize(stmt);
    return list;
}

domain::Drink SqliteDrinkRepository::create(const domain::Drink& drink) {
    const std::string sql = "INSERT INTO drinks(name, type_id, ingredients, recipe_text) VALUES(?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, drink.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, drink.type_id);
    const auto serialized_ingredients = ingredients_json(drink.ingredients);
    sqlite3_bind_text(stmt, 3, serialized_ingredients.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, drink.recipe_text.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err_msg = sqlite3_errmsg(con_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("sqlite insert failure: " + err_msg);
    }

    sqlite3_finalize(stmt);

    auto created = get_by_id(static_cast<int>(sqlite3_last_insert_rowid(con_.handle())));
    if (!created)
        throw std::runtime_error("drink was inserted but could not be retrieved");
    return *created;
}

std::optional<domain::Drink> SqliteDrinkRepository::update(const int& id, const domain::Drink& drink) {
    const std::string sql = "UPDATE drinks SET name = ?, type_id = ?, ingredients = ?, recipe_text = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, drink.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, drink.type_id);
    const auto serialized_ingredients = ingredients_json(drink.ingredients);
    sqlite3_bind_text(stmt, 3, serialized_ingredients.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, drink.recipe_text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, id);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    if (!ok) {
        return std::nullopt;
    }

    return get_by_id(id);
}

bool SqliteDrinkRepository::remove(const int& id) {
    const std::string sql = "DELETE FROM drinks WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(con_.handle()) > 0;
}

}