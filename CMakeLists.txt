cmake_minimum_required(VERSION 3.5)

project(lqxx)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

if (NOT TARGET tasklistsWorker)
    add_subdirectory(tasklists)
endif()

if (NOT TARGET tasksWorker)
    add_subdirectory(tasks)
endif()

# add neo4j
if (NOT DEFINED NEO_CLIENT_DIR)
    set(NEO_CLIENT_DIR /opt/homebrew/Cellar/libneo4j-client/2.2.0)
endif (NOT DEFINED NEO_CLIENT_DIR)

# main executable file
add_executable(lqxx ${PROJECT_SOURCE_DIR}/main.cpp)
target_link_libraries(lqxx PRIVATE api DB.o users gtest oauthcpp nlohmann_json cpp-jwt neo4j-client tasklistsWorker tasksWorker)
