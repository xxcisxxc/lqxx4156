cmake_minimum_required(VERSION 3.5)

project(users)

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

# google test
if (NOT TARGET gtest)
    add_subdirectory(${EXTERNAL_DIR}/googletest ${EXTERNAL_BIN_DIR}/googletest)
endif()

# users object
add_library(users OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/users.cpp)
target_include_directories(users PUBLIC ${ROOT_DIR})

# users unit test
add_executable(test_users ${CMAKE_CURRENT_SOURCE_DIR}/test/test_users.cpp)
target_include_directories(test_users PRIVATE ${EXTERNAL_DIR}/googletest/include)
target_link_libraries(test_users PRIVATE users gtest)
