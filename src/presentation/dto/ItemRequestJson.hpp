#pragma once
#include "application/dto/CreateItemRequest.hpp"
#include <boost/json.hpp>
#include <limits>
#include <stdexcept>

namespace presentation {

namespace json = boost::json;

inline application::CreateItemRequest
parse_create_item_request(const std::string& body) {
    auto value = boost::json::parse(body);
    const auto& obj = value.as_object();

    application::CreateItemRequest req;

    if (auto* t = obj.if_contains("name"))
        req.name = std::string(t->as_string());
    auto* type = obj.if_contains("type_id");
    if (!type || !type->is_int64())
        throw std::invalid_argument("type_id must be an integer");
    const auto type_id = type->as_int64();
    if (type_id <= 0 || type_id > std::numeric_limits<int>::max())
        throw std::invalid_argument("type_id must be a positive integer");
    req.type_id = static_cast<int>(type_id);
    if (auto* d = obj.if_contains("daytime_type_id")) {
        if (!d->is_int64() || d->as_int64() <= 0)
            throw std::invalid_argument("daytime_type_id must be a positive integer");
        req.daytime_type_id = static_cast<int>(d->as_int64());
    }
    if (auto* r = obj.if_contains("recipe_text"))
        req.recipe_text = std::string(r->as_string());

    if (auto* value = obj.if_contains("ingredients")) {
        if (!value->is_array())
            throw std::invalid_argument("ingredients must be an array of strings");
        for (const auto& ingredient : value->as_array()) {
            if (!ingredient.is_string())
                throw std::invalid_argument("ingredients must be an array of strings");
            req.ingredients.emplace_back(ingredient.as_string());
        }
    }

    // if (auto* arr = obj.if_contains("comments")) {
    //     for (const auto& v : arr->as_array())
    //         req.comments.emplace_back(std::string(v.as_string()));
    // }

    return req;
}
}
