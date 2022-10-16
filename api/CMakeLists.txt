cmake_minimum_required(VERSION 3.5)

project(api)

# project top root directory
if (NOT DEFINED ROOT_DIR)
    set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif (NOT DEFINED ROOT_DIR)

# external dependencies source code directory
if (NOT DEFINED EXTERNAL_DIR)
    set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external)
endif (NOT DEFINED EXTERNAL_DIR)

# directory to put dependencies binary
if (NOT DEFINED EXTERNAL_BIN_DIR)
    set(EXTERNAL_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../build/external)
endif (NOT DEFINED EXTERNAL_BIN_DIR)

# 3 dependencies to build api test

# add json
if (NOT TARGET nlohmann_json)
    add_subdirectory(${EXTERNAL_DIR}/json ${EXTERNAL_BIN_DIR}/json)
endif()

# add gtest
if (NOT TARGET gtest)
    add_subdirectory(${EXTERNAL_DIR}/googletest ${EXTERNAL_BIN_DIR}/googletest)
endif()

# add oauth
if (NOT TARGET oauthcpp)
    add_subdirectory(${EXTERNAL_DIR}/liboauthcpp/build ${EXTERNAL_BIN_DIR}/liboauthcpp/build)
endif()

# add users, because mock need it
if (NOT TARGET users)
    add_subdirectory(${ROOT_DIR}/users ${ROOT_DIR}/build/users)
endif()

# api object
add_library(api OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/api.cpp)
target_include_directories(api PUBLIC ${EXTERNAL_DIR})
target_include_directories(api PUBLIC ${EXTERNAL_DIR}/json/include)
target_include_directories(api PUBLIC ${ROOT_DIR})

# api test executable
add_executable(test_api ${CMAKE_CURRENT_SOURCE_DIR}/test/test_api.cpp)
target_include_directories(test_api PRIVATE ${EXTERNAL_DIR}/googletest/include)
target_link_libraries(test_api PRIVATE api gtest oauthcpp users)
