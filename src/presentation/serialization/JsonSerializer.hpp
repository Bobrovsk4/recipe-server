#pragma once
#include <string>
#include <stdexcept>
#include "domain/entities/Item.hpp"

namespace presentation {

inline std::string ttos(domain::TYPES t) {
switch (t) {
        case domain::TYPES::Asian:   return "asian";
        case domain::TYPES::Western: return "western";
        case domain::TYPES::Russian: return "russian";
        case domain::TYPES::Chinese: return "chinese";
        case domain::TYPES::None:    return "none";
    }
    return "none";
}

inline domain::TYPES stot(const std::string& s) {
    if (s == "asian")   return domain::TYPES::Asian;
    if (s == "western") return domain::TYPES::Western;
    if (s == "russian") return domain::TYPES::Russian;
    if (s == "chinese") return domain::TYPES::Chinese;
    if (s == "none")    return domain::TYPES::None;
    throw std::invalid_argument("bad item type: " + s);
}

}