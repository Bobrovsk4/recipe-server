#pragma once

#include "application/dto/CreateDrinkRequest.hpp"
#include "application/dto/CreateDrinkResponse.hpp"
#include "domain/repositories/IDrinkRepository.hpp"

#include "shared/Result.hpp"

namespace application {

using namespace shared;

class DrinkUseCase {
public:
    explicit DrinkUseCase(domain::IDrinkRepository& repo) : repo_(repo) {}

    Result<domain::Drink>      get_by_id(const int& id);
    std::vector<domain::Drink> list();
    std::vector<std::pair<int, std::string>> list_types();
    std::vector<domain::Drink> list_by_type(const std::string& t);
    Result<CreateDrinkResponce> create(const CreateDrinkRequest& req);
    Result<domain::Drink>       update(const int& id, const CreateDrinkRequest& req);
    Result<bool>                remove(const int& id);

private:
    domain::IDrinkRepository& repo_;
};

}
