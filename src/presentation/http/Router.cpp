#include "presentation/http/Router.hpp"
#include <sstream>
#include <iostream>

namespace presentation {

namespace {

std::vector<std::string> split_path(const std::string& path) {
    std::vector<std::string> out;
    std::stringstream ss(path);
    std::string part;
    while (std::getline(ss, part, '/'))
        if (!part.empty()) out.push_back(part);
    return out;
}

bool match(const std::vector<std::string>& pattern,
           const std::vector<std::string>& actual,
           std::vector<std::string>& params)
{
    if (pattern.size() != actual.size()) return false;
    params.clear();
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        if (!pattern[i].empty() && pattern[i][0] == ':')
            params.push_back(actual[i]);
        else if (pattern[i] != actual[i])
            return false;
    }
    return true;
}

}

void Router::add(http::verb method, const std::string& pattern, Handler handler) {
    routes_.push_back({method, split_path(pattern), std::move(handler)});
}

Response Router::dispatch(const Request& req) const {
    const auto target = std::string(req.target().data(), req.target().size());
    const auto method = std::string(http::to_string(req.method()));

    std::cout << "[req] " << method << " " << target << "\n";

    const auto actual = split_path(target);
    for (const auto& r : routes_) {
        if (r.method != req.method()) continue;
        std::vector<std::string> params;
        if (match(r.segments, actual, params)) {
            auto res = r.handler(req, params);
            std::cout << "[res] " << method << " " << target
                      << " -> " << res.result_int()
                      << " body=" << res.body() << "\n";
            return res;
        }
    }
    std::cout << "[res] " << method << " " << target << " -> 404\n";
    return make_json(http::status::not_found, R"({"error":"not found"})");
}

Response Router::make_json(http::status status, const std::string& body) {
    Response res{status, 11};
    res.set(http::field::content_type, "application/json");
    res.body() = body;
    res.prepare_payload();
    return res;
}

}