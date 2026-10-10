#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <iostream>

#include "application/use_cases/items/ItemUseCase.hpp"
#include "infrastructure/persistence/sqlite/SqliteConnection.hpp"
#include "infrastructure/persistence/sqlite/SqliteItemRepository.hpp"
#include "presentation/controllers/ItemController.hpp"
#include "presentation/http/Router.hpp"
#include "presentation/http/Server.hpp"

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
                daytime_type_id INTEGER NOT NULL REFERENCES daytime_types(id),
                ingredients     TEXT[],
                recipe_text     TEXT NOT NULL
            );

            INSERT OR IGNORE INTO daytime_types (id, name)
            VALUES (1,'завтрак'), (2,'обед'), (3,'ужин');
        )");
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
