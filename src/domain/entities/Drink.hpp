#pragma once

#include <optional>
#include <string>
#include <vector>

namespace domain {

struct Drink {
    std::optional<int> id;
    std::string name;
    int type_id;
    std::vector<std::string> ingredients;
    std::string recipe_text;
    // TODO: добавить изображение
};

}
