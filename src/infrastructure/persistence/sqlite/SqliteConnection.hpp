#pragma once

#include <sqlite3.h>
#include <string>

namespace infrastructure {

class SqliteConnection {
public:
    explicit SqliteConnection(const std::string& path);
    ~SqliteConnection();

    SqliteConnection(const SqliteConnection&)             = delete;
    SqliteConnection& operator =(const SqliteConnection&) = delete;

    sqlite3* handle() const { return db_; }
    void execute(const std::string& sql);
private:
    sqlite3* db_ = nullptr;
};

}