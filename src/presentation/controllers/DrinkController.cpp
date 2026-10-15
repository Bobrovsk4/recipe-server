#include "presentation/controllers/DrinkController.hpp"
#include "presentation/dto/ErrorResponse.hpp"
#include "presentation/dto/DrinkRequestJson.hpp"
#include "presentation/dto/DrinkResponseJson.hpp"

#include <boost/json.hpp>
#include <optional>
#include <random>

namespace presentation {

namespace {

std::optional<int> to_int(const std::string& s) {
    try {
        std::size_t parsed = 0;
        const int value = std::stoi(s, &parsed);
        if (parsed != s.size() || value <= 0)
            return std::nullopt;
        return value;
    } catch (...) {
        return std::nullopt;
    }
}

}

void DrinkController::register_routes(Router& r) {
    using namespace boost::beast::http;
    namespace json = boost::json;

    // GET /api/drink_types
    r.add(verb::get, "/api/drink_types",
        [this](const Request&,
           const std::vector<std::string>&,
           const QueryParams&)
    {
        json::array arr;
        for (const auto& type : drinks_.list_types()) {
            arr.push_back(json::object{
                {"id", type.first},
                {"name", type.second}
            });
        }
        return Router::make_json(status::ok, json::serialize(arr));
    });

    // GET /api/drinks[?type=N]
    r.add(verb::get, "/api/drinks",
        [this](const Request&,
               const std::vector<std::string>&,
               const QueryParams& query)
    {
        std::optional<int> filter_type;
        if (auto it = query.find("type"); it != query.end()) {
            filter_type = to_int(it->second);
            if (!filter_type)
                return Router::make_json(status::bad_request,
                                          error_json("bad type"));
        }

        json::array arr;
        if (filter_type) {
            for (const auto& d : drinks_.list_by_type(std::to_string(*filter_type)))
                arr.push_back(to_json(d));
        } else {
            for (const auto& d : drinks_.list())
                arr.push_back(to_json(d));
        }
        return Router::make_json(status::ok, json::serialize(arr));
    });

    // GET /api/drinks/random[?type_id=N]
    r.add(verb::get, "/api/drinks/random",
        [this](const Request&,
               const std::vector<std::string>&,
               const QueryParams& query) {
            std::optional<int> type_id;
            if (auto it = query.find("type_id"); it != query.end()) {
                type_id = to_int(it->second);
                if (!type_id)
                    return Router::make_json(status::bad_request, error_json("bad type_id"));
            }

            auto all = type_id ? drinks_.list_by_type(std::to_string(*type_id))
                               : drinks_.list();
            if (all.empty())
                return Router::make_json(status::not_found,
                                        error_json("no drinks available"));

            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<size_t> dist(0, all.size() - 1);
            const auto& drink = all[dist(rng)];
            return Router::make_json(status::ok, to_json_string(drink));
        });

    // GET /api/drinks/:id
    r.add(verb::get, "/api/drinks/:id",
        [this](const Request&,
               const std::vector<std::string>& p,
               const QueryParams&) {
            auto id = to_int(p[0]);
            if (!id)
                return Router::make_json(status::bad_request, error_json("bad id"));

            auto res = drinks_.get_by_id(*id);
            if (!res.has_value())
                return Router::make_json(status::not_found, error_json(res.error()));

            return Router::make_json(status::ok, to_json_string(res.value()));
        });

    // POST /api/drinks
    r.add(verb::post, "/api/drinks",
        [this](const Request& req,
               const std::vector<std::string>&,
               const QueryParams&) {
            try {
                auto req_dto = parse_create_drink_request(req.body());
                auto res     = drinks_.create(req_dto);
                if (!res.has_value())
                    return Router::make_json(status::bad_request, error_json(res.error()));
                return Router::make_json(status::created, to_json_string(res.value()));
            } catch (const std::exception& e) {
                return Router::make_json(status::bad_request, error_json(e.what()));
            }
        });

    // PUT /api/drinks/:id
    r.add(verb::put, "/api/drinks/:id",
        [this](const Request& req,
               const std::vector<std::string>& p,
               const QueryParams&) {
            auto id = to_int(p[0]);
            if (!id)
                return Router::make_json(status::bad_request, error_json("bad id"));
            try {
                auto req_dto = parse_create_drink_request(req.body());
                auto res     = drinks_.update(*id, req_dto);
                if (!res.has_value())
                    return Router::make_json(status::not_found, error_json(res.error()));
                return Router::make_json(status::ok, to_json_string(res.value()));
            } catch (const std::exception& e) {
                return Router::make_json(status::bad_request, error_json(e.what()));
            }
        });

    // DELETE /api/drinks/:id
    r.add(verb::delete_, "/api/drinks/:id",
        [this](const Request&,
               const std::vector<std::string>& p,
               const QueryParams&) {
            auto id = to_int(p[0]);
            if (!id)
                return Router::make_json(status::bad_request, error_json("bad id"));

            auto res = drinks_.remove(*id);
            if (!res.has_value())
                return Router::make_json(status::not_found, error_json(res.error()));
            return Router::make_json(status::no_content, "");
        });
}

}
