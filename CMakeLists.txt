cmake_minimum_required(VERSION 3.16)
project(backend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.70 CONFIG REQUIRED COMPONENTS system json)
find_package(SQLite3 REQUIRED)

# ---------- application ----------
add_library(application STATIC
    src/application/use_cases/items/ItemUseCase.cpp
)
target_include_directories(application PUBLIC ${CMAKE_SOURCE_DIR}/src)

# ---------- infrastructure ----------
add_library(infrastructure STATIC
    src/infrastructure/persistence/sqlite/SqliteConnection.cpp
    src/infrastructure/persistence/sqlite/SqliteItemRepository.cpp
)
target_include_directories(infrastructure PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(infrastructure PUBLIC SQLite3::SQLite3)

# ---------- presentation ----------
add_library(presentation STATIC
    src/presentation/controllers/ItemController.cpp
    src/presentation/http/Router.cpp
    src/presentation/http/Session.cpp
    src/presentation/http/Server.cpp
)
target_include_directories(presentation PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(presentation PUBLIC
    application
    Boost::system
    Boost::json
)

# ---------- backend ----------
add_executable(backend src/main.cpp)
target_link_libraries(backend PRIVATE
    application
    infrastructure
    presentation
)