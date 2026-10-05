#pragma once

#include "domain/repositories/IItemRepository.hpp"
#include "infrastructure/persistence/sqlite/SqliteConnection.hpp"

namespace infrastructure {

class SqliteItemRepository: public domain::IItemRepository {
public:
    explicit SqliteItemRepository(SqliteConnection& con);

    std::optional<domain::Item> get_by_id(const int& id);
    std::vector<domain::Item>   list();
    std::vector<domain::Item>   list_by_type(domain::TYPES t);
    domain::Item                create(const domain::Item& item);
    std::optional<domain::Item> update(const int& id, const domain::Item& item);
    bool                        remove(const int& id);
private:
    SqliteConnection& con_;
};

}
