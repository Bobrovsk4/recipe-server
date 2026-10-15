#pragma once

#include "domain/entities/Drink.hpp"
#include <string>
#include <vector>

namespace application {

struct CreateDrinkRequest {
    std::string name;
    int type_id = 0;
    std::vector<std::string> ingredients;
    std::string recipe_text;

    domain::Drink to_domain() const {
        return domain::Drink{std::nullopt, name, type_id, ingredients, recipe_text};
    }
};

}
