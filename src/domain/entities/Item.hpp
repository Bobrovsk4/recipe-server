#pragma once

#include <optional>
#include <string>
#include <vector>

namespace domain {

enum TYPES {
    Asian,
    Western,
    Russian,
    Chinese,
    None
};

struct Item {
    std::optional<int> id;

    TYPES type;
    std::string recipe_text;
    // std::vector<std::string> comments;
    // TODO: добавить изображение
};

}