#pragma once

#include "domain/entities/Drink.hpp"
#include <optional>
#include <string>

namespace domain {

class DrinkValidator {
public:
    static std::optional<std::string> validate(const Drink& drink) {
        if (drink.name.empty()) {
            return "Empty name";
        }
        if (drink.recipe_text.empty()) {
            return "Empty recipe";
        }
        return std::nullopt;
    }
};

}
