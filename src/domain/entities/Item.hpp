#pragma once

#include <optional>
#include <string>
#include <vector>

namespace domain {

struct Item {
    std::optional<int> id;
    std::string name;
    int type_id;
    int daytime_type_id;
    std::vector<std::string> ingredients;
    std::string recipe_text;
    // std::vector<std::string> comments;
    // TODO: добавить изображение
};

}
