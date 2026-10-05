#include <stdexcept>
#include "infrastructure/persistence/sqlite/SqliteItemRepository.hpp"

namespace infrastructure {

namespace {

int ttoi(domain::TYPES t) {
    switch (t) {
        case domain::TYPES::Asian:   return 0;
        case domain::TYPES::Western: return 1;
        case domain::TYPES::Russian: return 2;
        case domain::TYPES::Chinese: return 3;
        case domain::TYPES::None:    return 4;
    }
    return 4;
}

domain::TYPES itot(int v) {
    switch (v) {
        case 0:  return domain::TYPES::Asian;
        case 1:  return domain::TYPES::Western;
        case 2:  return domain::TYPES::Russian;
        case 3:  return domain::TYPES::Chinese;
        default: return domain::TYPES::None;
    }
}

}

std::string select = "SELECT * FROM items";

SqliteItemRepository::SqliteItemRepository(SqliteConnection& con) : con_(con) {}

std::optional<domain::Item> SqliteItemRepository::get_by_id(const int& id) {
    const std::string sql = select + " WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    

    std::optional<domain::Item> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = domain::Item{
            sqlite3_column_int(stmt, 0),
            itot(atoi(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)))),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
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
            itot(atoi(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)))),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
        });
    }
    
    sqlite3_finalize(stmt);
    return list;
}

std::vector<domain::Item>   SqliteItemRepository::list_by_type(domain::TYPES t) {
    std::vector<domain::Item> list;
    const int type = ttoi(t);

    std::string select_by_type = select + " WHERE type = ?";
    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(con_.handle(), select_by_type.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, type);
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        list.push_back(domain::Item{
            sqlite3_column_int(stmt, 0),
            itot(atoi(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)))),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
        });
    }

    sqlite3_finalize(stmt);
    return list;
}

domain::Item                SqliteItemRepository::create(const domain::Item& item) {
    const std::string sql = "INSERT INTO items(type, recipe_text) VALUES(?, ?)";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, ttoi(item.type));
    sqlite3_bind_text(stmt, 2, item.recipe_text.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err_msg = sqlite3_errmsg(con_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("sqlite insert failure: " + err_msg);
    }

    sqlite3_finalize(stmt);

    domain::Item copy = item;
    copy.id = static_cast<int>(sqlite3_last_insert_rowid(con_.handle()));
    return copy;
}

std::optional<domain::Item> SqliteItemRepository::update(const int& id, const domain::Item& item) {
    const std::string sql = "UPDATE items SET type = ?, recipe_text = ? WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(con_.handle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, ttoi(item.type));
    sqlite3_bind_text(stmt, 2, item.recipe_text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, id);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    if (!ok) {
        return std::nullopt;
    }

    domain::Item copy = item;
    copy.id = id;
    return copy;
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