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
    std::vector<std::pair<int, std::string>> list_types();
    Result<std::pair<int, std::string>> create_type(const std::string& name);
    Result<std::pair<int, std::string>> update_type(const int& id, const std::string& name);
    Result<bool>               remove_type(const int& id);
    std::vector<domain::Item>  list_by_type(const std::string& t);
    std::vector<domain::Item>  list_by_filters(const std::optional<int>& type_id,
                                               const std::optional<int>& daytime_type_id);
    Result<CreateItemResponce> create(const CreateItemRequest& req);
    Result<domain::Item>       update(const int& id, const CreateItemRequest& req);
    Result<bool>               remove(const int& id);

private:
    domain::IItemRepository& repo_;
};

}
