#include "presentation/http/Session.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>

namespace presentation {

Session::Session(tcp::socket socket, const Router& router)
    : socket_(std::move(socket)), router_(router) {}

void Session::run() { do_read(); }

void Session::do_read() {
    auto self = shared_from_this();
    http::async_read(socket_, buffer_, req_,
        [self](beast::error_code ec, std::size_t) { self->on_read(ec); });
}

void Session::on_read(beast::error_code ec) {
    if (ec) {
        if (ec != net::error::eof && ec != http::error::end_of_stream)
            std::cerr << "Request read error: " << ec.message() << '\n';
        return;
    }

    const auto request_started = std::chrono::steady_clock::now();
    std::clog << "HTTP request method=" << req_.method_string()
              << " target=" << std::quoted(std::string(req_.target().data(), req_.target().size()))
              << " body=" << std::quoted(req_.body()) << '\n';

    Response res;
    try {
        res = router_.dispatch(req_);
    } catch (const std::exception& e) {
        std::cerr << "Request processing error for " << req_.method_string() << ' '
                  << req_.target() << ": " << e.what() << '\n';
        res = Router::make_json(http::status::internal_server_error,
                                R"({"error":"internal server error"})");
    } catch (...) {
        std::cerr << "Unknown request processing error for " << req_.method_string()
                  << ' ' << req_.target() << '\n';
        res = Router::make_json(http::status::internal_server_error,
                                R"({"error":"internal server error"})");
    }

    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - request_started).count();
    std::clog << "HTTP response status=" << res.result_int()
              << " body=" << std::quoted(res.body())
              << " duration_ms=" << elapsed << '\n';
    res.version(req_.version());
    res.keep_alive(false);
    do_write(std::move(res));
}

void Session::do_write(Response res) {
    auto self = shared_from_this();
    auto sp   = std::make_shared<Response>(std::move(res));
    http::async_write(socket_, *sp,
        [self, sp](beast::error_code ec, std::size_t) {
            if (ec) return;
            beast::error_code ignored;
            self->socket_.shutdown(tcp::socket::shutdown_send, ignored);
        });
}

}
