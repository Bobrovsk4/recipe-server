#pragma once
#include "application/dto/CreateItemRequest.hpp"
#include "presentation/serialization/JsonSerializer.hpp"
#include <boost/json.hpp>

namespace presentation {

namespace json = boost::json;

inline application::CreateItemRequest
parse_create_item_request(const std::string& body) {
    auto value = boost::json::parse(body);
    const auto& obj = value.as_object();

    application::CreateItemRequest req;

    if (auto* t = obj.if_contains("type"))
        req.type = stot(std::string(t->as_string()));
    if (auto* r = obj.if_contains("recipe_text"))
        req.recipe_text = std::string(r->as_string());

    // if (auto* arr = obj.if_contains("comments")) {
    //     for (const auto& v : arr->as_array())
    //         req.comments.emplace_back(std::string(v.as_string()));
    // }

    return req;
}
}