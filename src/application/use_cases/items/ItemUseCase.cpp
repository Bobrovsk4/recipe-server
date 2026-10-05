#include "application/use_cases/items/ItemUseCase.hpp"
#include "domain/services/ItemValidator.hpp"

namespace application {

using namespace shared;

Result<domain::Item>       ItemUseCase::get_by_id(const int& id) {
    auto item = repo_.get_by_id(id);
    if (!item) return Result<domain::Item>::err("not found");

    return Result<CreateItemResponce>::ok(item.value());
}

std::vector<domain::Item>  ItemUseCase::list() {
    return repo_.list();
}

std::vector<domain::Item>  ItemUseCase::list_by_type(const domain::TYPES& t) {
    return repo_.list_by_type(t);
}

Result<CreateItemResponce> ItemUseCase::create(const CreateItemRequest& req) {
    auto item = req.to_domain();
    if (auto err = domain::ItemValidator::validate(item)) {
        return Result<CreateItemResponce>::err(err.value());
    }

    return Result<CreateItemResponce>::ok(repo_.create(item));
}

Result<domain::Item>       ItemUseCase::update(const int& id, const CreateItemRequest& req) {
    auto item = req.to_domain();
    if (auto err = domain::ItemValidator::validate(item)) {
        return Result<domain::Item>::err(err.value());
    }

    auto updated = repo_.update(id, item);
    if (!updated) {
        return Result<domain::Item>::err("not found");
    }

    return Result<domain::Item>::ok(updated.value());
}

Result<bool>               ItemUseCase::remove(const int& id) {
    if (!repo_.remove(id)) {
        return Result<bool>::err("not found");
    }
    return Result<bool>::ok(true);
}

}