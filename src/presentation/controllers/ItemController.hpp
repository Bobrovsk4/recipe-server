#pragma once
#include "application/use_cases/items/ItemUseCase.hpp"
#include "presentation/http/Router.hpp"

namespace presentation {

class ItemController {
public:
    explicit ItemController(application::ItemUseCase& items) : items_(items) {}

    void register_routes(Router& router);

private:
    application::ItemUseCase& items_;
};

}