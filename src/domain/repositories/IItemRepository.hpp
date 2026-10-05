#pragma once

#include <optional>
#include "domain/entities/Item.hpp"

namespace domain {

class IItemRepository {
public:
    virtual ~IItemRepository() = default;

    virtual std::optional<Item> get_by_id(const int& id)                  = 0;
    virtual std::vector<Item>   list()                                    = 0;
    virtual std::vector<Item>   list_by_type(TYPES t)                     = 0;
    virtual Item                create(const Item& item)                  = 0;
    virtual std::optional<Item> update(const int& id, const Item& item)   = 0;
    virtual bool                remove(const int& id)                     = 0;
};

}