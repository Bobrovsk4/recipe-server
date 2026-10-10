#pragma once

#include "domain/entities/Item.hpp"
#include <string>
#include <vector>

namespace application {

struct CreateItemRequest {
    std::string name;
    int type_id = 0;
    int daytime_type_id = 4;
    std::vector<std::string> ingredients;
    std::string recipe_text;
    // std::vector<std::string> comments;

    domain::Item to_domain() const {
        return domain::Item{std::nullopt, name, type_id, daytime_type_id, ingredients, recipe_text};
    }
};

}
