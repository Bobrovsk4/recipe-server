#include "presentation/http/Server.hpp"
#include "presentation/http/Listener.hpp"

namespace presentation {

Server::Server(net::io_context& ioc,
               const tcp::endpoint& endpoint,
               const Router& router)
    : ioc_(ioc), endpoint_(endpoint), router_(router) {}

void Server::run() {
    std::make_shared<Listener>(ioc_, endpoint_, router_)->run();
}

}