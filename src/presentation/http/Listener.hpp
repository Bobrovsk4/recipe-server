#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

#include "presentation/http/Session.hpp"

namespace presentation {

class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(net::io_context& ioc,
             const tcp::endpoint& endpoint,
             const Router& router)
        : ioc_(ioc), acceptor_(net::make_strand(ioc)), router_(router)
    {
        beast::error_code ec;
        acceptor_.open(endpoint.protocol(), ec);
        if (ec) throw beast::system_error(ec);

        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) throw beast::system_error(ec);

        acceptor_.bind(endpoint, ec);
        if (ec) throw beast::system_error(ec);

        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec) throw beast::system_error(ec);
    }

    void run() { do_accept(); }

private:
    void do_accept() {
        acceptor_.async_accept(net::make_strand(ioc_),
            [self = shared_from_this()](beast::error_code ec, tcp::socket socket) {
                if (!ec)
                    std::make_shared<Session>(
                        std::move(socket), self->router_)->run();
                self->do_accept();
            });
    }

    net::io_context&  ioc_;
    tcp::acceptor     acceptor_;
    const Router&     router_;
};

}