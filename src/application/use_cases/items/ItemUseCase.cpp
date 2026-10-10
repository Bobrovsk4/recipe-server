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

std::vector<std::pair<int, std::string>> ItemUseCase::list_types() {
    return repo_.list_types();
}

std::vector<std::pair<int, std::string>> ItemUseCase::list_daytime_types() {
    return repo_.list_daytime_types();
}

Result<std::pair<int, std::string>> ItemUseCase::create_type(const std::string& name) {
    if (name.empty()) return Result<std::pair<int, std::string>>::err("name is required");
    try {
        return Result<std::pair<int, std::string>>::ok(repo_.create_type(name));
    } catch (const std::exception& e) {
        return Result<std::pair<int, std::string>>::err(e.what());
    }
}

Result<std::pair<int, std::string>> ItemUseCase::update_type(const int& id, const std::string& name) {
    if (name.empty()) return Result<std::pair<int, std::string>>::err("name is required");
    try {
        auto updated = repo_.update_type(id, name);
        if (!updated) return Result<std::pair<int, std::string>>::err("not found");
        return Result<std::pair<int, std::string>>::ok(*updated);
    } catch (const std::exception& e) {
        return Result<std::pair<int, std::string>>::err(e.what());
    }
}

Result<bool> ItemUseCase::remove_type(const int& id) {
    try {
        if (!repo_.remove_type(id)) return Result<bool>::err("not found");
        return Result<bool>::ok(true);
    } catch (const std::exception& e) {
        return Result<bool>::err(e.what());
    }
}

std::vector<domain::Item>  ItemUseCase::list_by_type(const std::string& t) {
    return repo_.list_by_type(t);
}

std::vector<domain::Item> ItemUseCase::list_by_filters(
    const std::optional<int>& type_id,
    const std::optional<int>& daytime_type_id) {
    return repo_.list_by_filters(type_id, daytime_type_id);
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
