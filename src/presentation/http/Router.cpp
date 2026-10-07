#include "presentation/http/Router.hpp"
#include <sstream>

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

QueryParams parse_query(const std::string& q) {
    QueryParams out;
    std::stringstream ss(q);
    std::string pair;
    while (std::getline(ss, pair, '&')) {
        if (pair.empty()) continue;
        auto eq = pair.find('=');
        if (eq == std::string::npos)
            out[pair] = "";
        else
            out[pair.substr(0, eq)] = pair.substr(eq + 1);
    }
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

    auto qpos  = target.find('?');
    auto path  = (qpos == std::string::npos) ? target : target.substr(0, qpos);
    auto query = (qpos == std::string::npos)
                     ? std::string{}
                     : target.substr(qpos + 1);

    const auto actual = split_path(path);
    const auto qparams = parse_query(query);

    for (const auto& r : routes_) {
        if (r.method != req.method()) continue;
        std::vector<std::string> params;
        if (match(r.segments, actual, params))
            return r.handler(req, params, qparams);
    }
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