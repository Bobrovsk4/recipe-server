#pragma once
#include "application/use_cases/drinks/DrinkUseCase.hpp"
#include "presentation/http/Router.hpp"

namespace presentation {

class DrinkController {
public:
    explicit DrinkController(application::DrinkUseCase& drinks) : drinks_(drinks) {}

    void register_routes(Router& router);

private:
    application::DrinkUseCase& drinks_;
};

}
