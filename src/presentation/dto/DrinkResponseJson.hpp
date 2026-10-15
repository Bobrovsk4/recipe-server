#pragma once
#include "domain/entities/Drink.hpp"
#include <boost/json.hpp>

namespace presentation {

namespace json = boost::json;

inline boost::json::object to_json(const domain::Drink& drink) {
    boost::json::object obj;
    obj["id"]       = drink.id.value_or(0);
    obj["name"]     = drink.name;
    obj["type_id"]  = drink.type_id;
    boost::json::array ingredients;
    for (const auto& ingredient : drink.ingredients)
        ingredients.emplace_back(ingredient);
    obj["ingredients"] = std::move(ingredients);
    obj["recipe_text"] = drink.recipe_text;

    return obj;
}

inline std::string to_json_string(const domain::Drink& drink) {
    return boost::json::serialize(to_json(drink));
}

}
