cmake_minimum_required(VERSION 3.5)

project(lqxx)

option(LQXX_TESTS "Configure CMake to build tests (or not)" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ROOT_DIR ${CMAKE_SOURCE_DIR})
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

find_package(nlohmann_json REQUIRED)
include_directories(${EXTERNAL_DIR}/cpp-jwt/include)
include_directories(${EXTERNAL_DIR}/cpp-httplib)
include_directories(${EXTERNAL_DIR}/liboauthcpp/src)

add_subdirectory(db)
add_subdirectory(api)
add_subdirectory(tasks)
add_subdirectory(tasklists)
add_subdirectory(users)

# main executable file
add_executable(lqxx lqxx.cpp)
target_link_libraries(lqxx PUBLIC api DB users tasklistsWorker tasksWorker neo4j-client nlohmann_json pthread ssl crypto)

if(LQXX_TESTS)
    add_subdirectory(test)
endif()