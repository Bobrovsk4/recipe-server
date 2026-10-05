#pragma once

#include "domain/entities/Item.hpp"
#include <optional>
#include <string>

namespace domain {

class ItemValidator {
public:
    static std::optional<std::string> validate(const Item& item) {
        if (item.name.empty()) {
            return "Empty name";
        }
        if (item.recipe_text.empty()) {
            return "Empty recipe";
        }
        return std::nullopt;
    }
};

}