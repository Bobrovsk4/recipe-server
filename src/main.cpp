#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

#include "application/use_cases/items/ItemUseCase.hpp"
#include "infrastructure/persistence/sqlite/SqliteConnection.hpp"
#include "infrastructure/persistence/sqlite/SqliteItemRepository.hpp"
#include "presentation/controllers/ItemController.hpp"
#include "presentation/http/Router.hpp"
#include "presentation/http/Server.hpp"

namespace {

bool has_column(sqlite3* db, const std::string& table, const std::string& column) {
    const std::string sql = "PRAGMA table_info(" + table + ")";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("sqlite schema inspection failure: " + std::string(sqlite3_errmsg(db)));

    bool found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const auto* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (name && column == name) {
            found = true;
            break;
        }
    }
    sqlite3_finalize(stmt);
    return found;
}

}

int main() {
    try {
        // infrastructure
        infrastructure::SqliteConnection conn{"app.db"};
        conn.execute(R"(
            PRAGMA foreign_keys = ON;
            CREATE TABLE IF NOT EXISTS types (
                id              INTEGER PRIMARY KEY AUTOINCREMENT,
                name            TEXT NOT NULL UNIQUE
            );
            CREATE TABLE IF NOT EXISTS daytime_types (
                id              INTEGER PRIMARY KEY AUTOINCREMENT,
                name            TEXT NOT NULL UNIQUE
            );
            CREATE TABLE IF NOT EXISTS items (
                id              INTEGER PRIMARY KEY AUTOINCREMENT,
                name            TEXT NOT NULL,
                type_id         INTEGER NOT NULL REFERENCES types(id),
                daytime_type_id INTEGER NOT NULL DEFAULT 4 REFERENCES daytime_types(id),
                ingredients     TEXT NOT NULL DEFAULT '[]',
                recipe_text     TEXT NOT NULL
            );

            INSERT OR IGNORE INTO daytime_types (id, name)
            VALUES (1,'завтрак'), (2,'обед'), (3,'ужин'), (4, 'общее');
        )");
        if (!has_column(conn.handle(), "items", "daytime_type_id"))
            conn.execute("ALTER TABLE items ADD COLUMN daytime_type_id INTEGER NOT NULL DEFAULT 4");
        if (!has_column(conn.handle(), "items", "ingredients"))
            conn.execute("ALTER TABLE items ADD COLUMN ingredients TEXT NOT NULL DEFAULT '[]'");
        infrastructure::SqliteItemRepository item_repo{conn};

        // application
        application::ItemUseCase items{item_repo};

        // presentation
        presentation::Router router;
        presentation::ItemController item_controller{items};
        item_controller.register_routes(router);

        const auto address = boost::asio::ip::make_address("0.0.0.0");
        boost::asio::io_context ioc{1};
        presentation::Server server{ioc, {address, 8080}, router};
        server.run();

        std::cout << "Listening on http://0.0.0.0:8080\n";
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
