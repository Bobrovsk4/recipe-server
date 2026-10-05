#pragma once

#include "application/dto/CreateItemRequest.hpp"
#include "application/dto/CreateItemResponse.hpp"
#include "domain/repositories/IItemRepository.hpp"

#include "shared/Result.hpp"

namespace application {

using namespace shared;

class ItemUseCase {
public:
    explicit ItemUseCase(domain::IItemRepository& repo) : repo_(repo) {}

    Result<domain::Item>       get_by_id(const int& id);
    std::vector<domain::Item>  list();
    std::vector<domain::Item>  list_by_type(const domain::TYPES& t);
    Result<CreateItemResponce> create(const CreateItemRequest& req);
    Result<domain::Item>       update(const int& id, const CreateItemRequest& req);
    Result<bool>               remove(const int& id);

private:
    domain::IItemRepository& repo_;
};

}