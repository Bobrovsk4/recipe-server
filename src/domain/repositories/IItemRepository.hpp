#pragma once

#include <string>
#include <optional>
#include "domain/entities/Item.hpp"

namespace domain {

class IItemRepository {
public:
    virtual ~IItemRepository() = default;

    virtual std::optional<Item> get_by_id(const int& id)                  = 0;
    virtual std::vector<Item>   list()                                    = 0;
    virtual std::vector<std::pair<int, std::string>> list_types()          = 0;
    virtual std::pair<int, std::string> create_type(const std::string& name) = 0;
    virtual std::optional<std::pair<int, std::string>> update_type(const int& id, const std::string& name) = 0;
    virtual bool                remove_type(const int& id)                 = 0;
    virtual std::vector<Item>   list_by_type(const std::string& t)        = 0;
    virtual std::vector<Item>   list_by_filters(const std::optional<int>& type_id,
                                                const std::optional<int>& daytime_type_id) = 0;
    virtual Item                create(const Item& item)                  = 0;
    virtual std::optional<Item> update(const int& id, const Item& item)   = 0;
    virtual bool                remove(const int& id)                     = 0;
};

}
