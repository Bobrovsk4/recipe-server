#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "presentation/http/Router.hpp"

namespace presentation {

class Server {
public:
    Server(boost::asio::io_context& ioc,
           const boost::asio::ip::tcp::endpoint& endpoint,
           const Router& router);

    void run();

private:
    boost::asio::io_context&        ioc_;
    boost::asio::ip::tcp::endpoint  endpoint_;
    const Router&                   router_;
};

}