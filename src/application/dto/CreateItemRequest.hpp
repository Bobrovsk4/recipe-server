#pragma once

#include "domain/entities/Item.hpp"
#include <string>
#include <vector>

namespace application {

struct CreateItemRequest {
    std::string name;
    int type_id = 0;
    std::string recipe_text;
    // std::vector<std::string> comments;

    domain::Item to_domain() const {
        return domain::Item{std::nullopt, name, std::to_string(type_id), recipe_text};
    }
};

}
