#pragma once

#include "domain/repositories/IItemRepository.hpp"
#include "infrastructure/persistence/sqlite/SqliteConnection.hpp"

namespace infrastructure {

class SqliteItemRepository: public domain::IItemRepository {
public:
    explicit SqliteItemRepository(SqliteConnection& con);

    std::optional<domain::Item> get_by_id(const int& id);
    std::vector<domain::Item>   list();
    std::vector<std::pair<int, std::string>> list_types();
    std::vector<std::pair<int, std::string>> list_daytime_types();
    std::pair<int, std::string> create_type(const std::string& name);
    std::optional<std::pair<int, std::string>> update_type(const int& id, const std::string& name);
    bool                        remove_type(const int& id);
    std::vector<domain::Item>   list_by_type(const std::string& t);
    std::vector<domain::Item>   list_by_filters(const std::optional<int>& type_id,
                                                const std::optional<int>& daytime_type_id);
    domain::Item                create(const domain::Item& item);
    std::optional<domain::Item> update(const int& id, const domain::Item& item);
    bool                        remove(const int& id);
private:
    SqliteConnection& con_;
};

}
