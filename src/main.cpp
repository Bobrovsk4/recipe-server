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
            CREATE TABLE IF NOT EXISTS items (
                id              INTEGER PRIMARY KEY AUTOINCREMENT,
                name            TEXT NOT NULL,
                type            TEXT NOT NULL,
                recipe_text    TEXT NOT NULL
            );
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