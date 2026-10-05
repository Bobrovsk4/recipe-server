#include "presentation/http/Session.hpp"

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
    if (ec) return;

    auto res = router_.dispatch(req_);
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