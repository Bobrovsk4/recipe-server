#pragma once
#include <boost/json.hpp>
#include <string>

namespace presentation {

namespace json = boost::json;

inline std::string error_json(const std::string& message) {
    boost::json::object obj;
    obj["error"] = message;
    return boost::json::serialize(obj);
}

}