#pragma once

#include "domain/repositories/IDrinkRepository.hpp"
#include "infrastructure/persistence/sqlite/SqliteConnection.hpp"

namespace infrastructure {

class SqliteDrinkRepository: public domain::IDrinkRepository {
public:
    explicit SqliteDrinkRepository(SqliteConnection& con);

    std::optional<domain::Drink> get_by_id(const int& id);
    std::vector<domain::Drink>   list();
    std::vector<std::pair<int, std::string>> list_types();
    std::vector<domain::Drink>   list_by_type(const std::string& t);
    domain::Drink                create(const domain::Drink& drink);
    std::optional<domain::Drink> update(const int& id, const domain::Drink& drink);
    bool                         remove(const int& id);
private:
    SqliteConnection& con_;
};

}
