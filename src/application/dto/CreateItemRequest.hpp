#pragma once

#include "domain/entities/Item.hpp"
#include <string>
#include <vector>

namespace application {

struct CreateItemRequest {
    domain::TYPES type;
    std::string recipe_text;
    // std::vector<std::string> comments;

    domain::Item to_domain() const {
        return domain::Item{std::nullopt, type, recipe_text};
    }
};

}
