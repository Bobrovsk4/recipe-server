#pragma once
#include <boost/beast/http.hpp>
#include <functional>
#include <string>
#include <vector>

namespace presentation {

namespace http = boost::beast::http;
using Request  = http::request<http::string_body>;
using Response = http::response<http::string_body>;

using Handler = std::function<
    Response(const Request&,
             const std::vector<std::string>& params,
             const QueryParams& query)>;

class Router {
public:
    void add(http::verb method, const std::string& pattern, Handler handler);
    Response dispatch(const Request& req) const;

    static Response make_json(http::status status, const std::string& body);

private:
    struct Route {
        http::verb               method;
        std::vector<std::string> segments;
        Handler                  handler;
    };
    std::vector<Route> routes_;
};

}
