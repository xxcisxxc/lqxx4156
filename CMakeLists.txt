cmake_minimum_required(VERSION 3.5)

project(lqxx)

# add submodules

if (NOT TARGET api)
    add_subdirectory(api)
endif()

if (NOT TARGET DB.o)
    add_subdirectory(db)
endif()

if (NOT TARGET users)
    add_subdirectory(users)
endif()

# main executable file
add_executable(lqxx ${PROJECT_SOURCE_DIR}/main.cpp)
target_link_libraries(lqxx PRIVATE api DB.o users gtest oauthcpp nlohmann_json cpp-jwt)
