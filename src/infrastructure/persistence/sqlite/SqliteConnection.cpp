#include "infrastructure/persistence/sqlite/SqliteConnection.hpp"
#include <stdexcept>

namespace infrastructure {

SqliteConnection::SqliteConnection(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string msg = db_ ? sqlite3_errmsg(db_) : "db open failed";
        sqlite3_close(db_);
        throw std::runtime_error("sqlite open error: " + msg);
    }
}

SqliteConnection::~SqliteConnection() {
    if (db_) sqlite3_close(db_);
}

void SqliteConnection::execute(const std::string& sql) {
    char *err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        throw std::runtime_error("sqlite exec error: " + msg);
    }
}

}