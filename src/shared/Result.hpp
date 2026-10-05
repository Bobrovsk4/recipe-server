#pragma once

#include <optional>
#include <string>
#include <utility>

namespace shared {

template <typename T>
class Result {
public:
    static Result ok(T value)              { return Result(std::move(value)); }
    static Result err(std::string message) { return Result(std::move(message), 0); }

    bool               has_value() const { return value_.has_value(); }
    const T&           value()     const { return *value_; }
    T&                 value()           { return *value_; }
    const std::string& error()     const { return error_; }

private:
    explicit Result(T value)              : value_(std::move(value)) {}
    explicit Result(std::string e, int)   : error_(std::move(e)) {}

    std::optional<T> value_;
    std::string      error_;
};

}