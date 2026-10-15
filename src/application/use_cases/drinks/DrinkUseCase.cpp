#include "application/use_cases/drinks/DrinkUseCase.hpp"
#include "domain/services/DrinkValidator.hpp"

namespace application {

using namespace shared;

Result<domain::Drink> DrinkUseCase::get_by_id(const int& id) {
    auto drink = repo_.get_by_id(id);
    if (!drink) return Result<domain::Drink>::err("not found");

    return Result<CreateDrinkResponce>::ok(drink.value());
}

std::vector<domain::Drink> DrinkUseCase::list() {
    return repo_.list();
}

std::vector<std::pair<int, std::string>> DrinkUseCase::list_types() {
    return repo_.list_types();
}

std::vector<domain::Drink> DrinkUseCase::list_by_type(const std::string& t) {
    return repo_.list_by_type(t);
}

Result<CreateDrinkResponce> DrinkUseCase::create(const CreateDrinkRequest& req) {
    auto drink = req.to_domain();
    if (auto err = domain::DrinkValidator::validate(drink)) {
        return Result<CreateDrinkResponce>::err(err.value());
    }

    return Result<CreateDrinkResponce>::ok(repo_.create(drink));
}

Result<domain::Drink> DrinkUseCase::update(const int& id, const CreateDrinkRequest& req) {
    auto drink = req.to_domain();
    if (auto err = domain::DrinkValidator::validate(drink)) {
        return Result<domain::Drink>::err(err.value());
    }

    auto updated = repo_.update(id, drink);
    if (!updated) {
        return Result<domain::Drink>::err("not found");
    }

    return Result<domain::Drink>::ok(updated.value());
}

Result<bool> DrinkUseCase::remove(const int& id) {
    if (!repo_.remove(id)) {
        return Result<bool>::err("not found");
    }
    return Result<bool>::ok(true);
}

}
