#pragma once
#include "domain/entities/Item.hpp"
#include "presentation/serialization/JsonSerializer.hpp"
#include <boost/json.hpp>

namespace presentation {

namespace json = boost::json;

inline boost::json::object to_json(const domain::Item& item) {
    boost::json::object obj;
    obj["id"]           = item.id.value_or(0);
    obj["type"]         = ttos(item.type);
    obj["recipe_text"] = item.recipe_text;

    // boost::json::array comments;
    // for (const auto& c : item.comments)
    //     comments.emplace_back(c);
    // obj["comments"] = std::move(comments);

    return obj;
}

inline std::string to_json_string(const domain::Item& item) {
    return boost::json::serialize(to_json(item));
}

}