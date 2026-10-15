#pragma once

#include <string>
#include <optional>
#include "domain/entities/Drink.hpp"

namespace domain {

class IDrinkRepository {
public:
    virtual ~IDrinkRepository() = default;

    virtual std::optional<Drink> get_by_id(const int& id)                  = 0;
    virtual std::vector<Drink>   list()                                    = 0;
    virtual std::vector<std::pair<int, std::string>> list_types()          = 0;
    virtual std::vector<Drink>   list_by_type(const std::string& t)        = 0;
    virtual Drink                create(const Drink& drink)                = 0;
    virtual std::optional<Drink> update(const int& id, const Drink& drink) = 0;
    virtual bool                remove(const int& id)                      = 0;
};

}
