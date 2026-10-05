#include "presentation/controllers/ItemController.hpp"
#include "presentation/dto/ErrorResponse.hpp"
#include "presentation/dto/ItemRequestJson.hpp"
#include "presentation/dto/ItemResponseJson.hpp"

#include <boost/json.hpp>
#include <optional>

namespace presentation {

namespace {

std::optional<int> to_int(const std::string& s) {
    try { return std::stoi(s); } catch (...) { return std::nullopt; }
}

}

void ItemController::register_routes(Router& r) {
    using namespace boost::beast::http;
    namespace json = boost::json;

    // GET /items
    r.add(verb::get, "/items",
        [this](const Request&, const std::vector<std::string>&) {
            json::array arr;
            for (const auto& it : items_.list())
                arr.push_back(to_json(it));
            return Router::make_json(status::ok, json::serialize(arr));
        });

    // GET /items/:id
    r.add(verb::get, "/items/:id",
        [this](const Request&, const std::vector<std::string>& p) {
            auto id = to_int(p[0]);
            if (!id)
                return Router::make_json(status::bad_request, error_json("bad id"));

            auto res = items_.get_by_id(*id);
            if (!res.has_value())
                return Router::make_json(status::not_found, error_json(res.error()));

            return Router::make_json(status::ok, to_json_string(res.value()));
        });

    // POST /items
    r.add(verb::post, "/items",
        [this](const Request& req, const std::vector<std::string>&) {
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

    // PUT /items/:id
    r.add(verb::put, "/items/:id",
        [this](const Request& req, const std::vector<std::string>& p) {
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

    // DELETE /items/:id
    r.add(verb::delete_, "/items/:id",
        [this](const Request&, const std::vector<std::string>& p) {
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