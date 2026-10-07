#pragma once

#include <optional>
#include <string>
#include <vector>

namespace domain {

struct Item {
    std::optional<int> id;
    std::string name;
    std::string type;
    std::string recipe_text;
    // std::vector<std::string> comments;
    // TODO: добавить изображение
};

}