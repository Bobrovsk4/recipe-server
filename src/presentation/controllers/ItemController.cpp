#include "presentation/controllers/ItemController.hpp"
#include "presentation/dto/ErrorResponse.hpp"
#include "presentation/dto/ItemRequestJson.hpp"
#include "presentation/dto/ItemResponseJson.hpp"

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

void ItemController::register_routes(Router& r) {
    using namespace boost::beast::http;
    namespace json = boost::json;

    // GET /api/types
    r.add(verb::get, "/api/types",
        [this](const Request&,
           const std::vector<std::string>&,
           const QueryParams&)
    {
        json::array arr;
        for (const auto& type : items_.list_types()) {
            arr.push_back(json::object{
                {"id", type.first},
                {"name", type.second}
            });
        }
        return Router::make_json(status::ok, json::serialize(arr));
    });

    // GET /api/items[?type=N&daytime_type=N]
    r.add(verb::get, "/api/items",
        [this](const Request&,
               const std::vector<std::string>&,
               const QueryParams& query)
    {
        std::optional<int> filter_type;
        std::optional<int> filter_daytime_type;
        if (auto it = query.find("type"); it != query.end()) {
            filter_type = to_int(it->second);
            if (!filter_type)
                return Router::make_json(status::bad_request,
                                          error_json("bad type"));
        }
        if (auto it = query.find("daytime_type"); it != query.end()) {
            filter_daytime_type = to_int(it->second);
            if (!filter_daytime_type)
                return Router::make_json(status::bad_request,
                                          error_json("bad daytime_type"));
        }

        json::array arr;
        for (const auto& it : items_.list_by_filters(filter_type, filter_daytime_type)) {
            arr.push_back(to_json(it));
        }
        return Router::make_json(status::ok, json::serialize(arr));
    });

    // GET /api/items/random
    r.add(verb::get, "/api/items/random",
        [this](const Request&,
               const std::vector<std::string>&,
               const QueryParams&) {
            auto all = items_.list();
            if (all.empty())
                return Router::make_json(status::not_found,
                                        error_json("no items available"));

            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<size_t> dist(0, all.size() - 1);
            const auto& item = all[dist(rng)];
            return Router::make_json(status::ok, to_json_string(item));
        });

    // GET /api/items/:id
    r.add(verb::get, "/api/items/:id",
        [this](const Request&,
               const std::vector<std::string>& p,
               const QueryParams&) {
            auto id = to_int(p[0]);
            if (!id)
                return Router::make_json(status::bad_request, error_json("bad id"));

            auto res = items_.get_by_id(*id);
            if (!res.has_value())
                return Router::make_json(status::not_found, error_json(res.error()));

            return Router::make_json(status::ok, to_json_string(res.value()));
        });

    // GET /api/daytime_types
    r.add(verb::get, "/api/daytime_types",
        [this](const Request&,
               const std::vector<std::string>& p,
               const QueryParams&) {
            json::array arr;
            for (const auto& t : items_.list_daytime_types()) {
                arr.push_back(json::object{{"id", t.first}, {"name", t.second}});
            }
            return Router::make_json(status::ok, json::serialize(arr));
        });

    // POST /api/types
    r.add(verb::post, "/api/types",
        [this](const Request& req,
               const std::vector<std::string>&,
               const QueryParams&) {
            try {
                const auto body = json::parse(req.body()).as_object();
                const auto* name = body.if_contains("name");
                if (!name || !name->is_string())
                    return Router::make_json(status::bad_request, error_json("name is required"));

                auto result = items_.create_type(std::string(name->as_string()));
                if (!result.has_value())
                    return Router::make_json(status::bad_request, error_json(result.error()));
                return Router::make_json(status::created, json::serialize(json::object{
                    {"id", result.value().first},
                    {"name", result.value().second}
                }));
            } catch (const std::exception& e) {
                return Router::make_json(status::bad_request, error_json(e.what()));
            }
        });

    // PUT /api/types/:id
    r.add(verb::put, "/api/types/:id",
        [this](const Request& req,
               const std::vector<std::string>& p,
               const QueryParams&) {
            auto id = to_int(p[0]);
            if (!id)
                return Router::make_json(status::bad_request, error_json("bad id"));
            try {
                const auto body = json::parse(req.body()).as_object();
                const auto* name = body.if_contains("name");
                if (!name || !name->is_string())
                    return Router::make_json(status::bad_request, error_json("name is required"));
                auto result = items_.update_type(*id, std::string(name->as_string()));
                if (!result.has_value()) {
                    const auto code = result.error() == "not found" ? status::not_found : status::conflict;
                    return Router::make_json(code, error_json(result.error()));
                }
                return Router::make_json(status::ok, json::serialize(json::object{
                    {"id", result.value().first},
                    {"name", result.value().second}
                }));
            } catch (const std::exception& e) {
                return Router::make_json(status::bad_request, error_json(e.what()));
            }
        });

    // DELETE /api/types/:id
    r.add(verb::delete_, "/api/types/:id",
        [this](const Request&,
               const std::vector<std::string>& p,
               const QueryParams&) {
            auto id = to_int(p[0]);
            if (!id)
                return Router::make_json(status::bad_request, error_json("bad id"));
            auto result = items_.remove_type(*id);
            if (!result.has_value()) {
                const auto code = result.error() == "not found" ? status::not_found : status::conflict;
                return Router::make_json(code, error_json(result.error()));
            }
            return Router::make_json(status::no_content, "");
        });

    // POST /api/items
    r.add(verb::post, "/api/items",
        [this](const Request& req,
               const std::vector<std::string>&,
               const QueryParams&) {
            try {
                auto req_dto = parse_create_item_request(req.body());
                auto res     = items_.create(req_dto);
                if (!res.has_value())
                    return Router::make_json(status::bad_request, error_json(res.error()));
                return Router::make_json(status::created, to_json_string(res.value()));
            } catch (const std::exception& e) {
                return Router::make_json(status::bad_request, error_json(e.what()));
            }
        });

    // PUT /api/items/:id
    r.add(verb::put, "/api/items/:id",
        [this](const Request& req,
               const std::vector<std::string>& p,
               const QueryParams&) {
            auto id = to_int(p[0]);
            if (!id)
                return Router::make_json(status::bad_request, error_json("bad id"));
            try {
                auto req_dto = parse_create_item_request(req.body());
                auto res     = items_.update(*id, req_dto);
                if (!res.has_value())
                    return Router::make_json(status::not_found, error_json(res.error()));
                return Router::make_json(status::ok, to_json_string(res.value()));
            } catch (const std::exception& e) {
                return Router::make_json(status::bad_request, error_json(e.what()));
            }
        });

    // DELETE /api/items/:id
    r.add(verb::delete_, "/api/items/:id",
        [this](const Request&,
               const std::vector<std::string>& p,
               const QueryParams&) {
            auto id = to_int(p[0]);
            if (!id)
                return Router::make_json(status::bad_request, error_json("bad id"));

            auto res = items_.remove(*id);
            if (!res.has_value())
                return Router::make_json(status::not_found, error_json(res.error()));
            return Router::make_json(status::no_content, "");
        });
}

}
