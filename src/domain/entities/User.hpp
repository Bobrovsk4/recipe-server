#pragma once

#include <optional>
#include <string>

namespace domain {

// TODO: реализовать авторизацию

struct User {
    std::optional<int> id;
    std::string name;
};

}