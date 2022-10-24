cmake_minimum_required(VERSION 3.5)

project(tasklists)

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

# add gtest
if (NOT TARGET gtest)
    add_subdirectory(${EXTERNAL_DIR}/googletest ${EXTERNAL_BIN_DIR}/googletest)
endif()

# add neo4j
if (NOT DEFINED NEO_CLIENT_DIR)
    set(NEO_CLIENT_DIR /opt/homebrew/Cellar/libneo4j-client/2.2.0)
endif (NOT DEFINED NEO_CLIENT_DIR)

add_library(tasklistsWorker OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/tasklistsWorker.cpp)
target_include_directories(tasklistsWorker PRIVATE ${NEO_CLIENT_DIR}/include)
target_include_directories(tasklistsWorker PUBLIC ${ROOT_DIR})

add_executable(test_tasklists ${CMAKE_CURRENT_SOURCE_DIR}/test/test_tasklists.cpp)
target_include_directories(test_tasklists PRIVATE ${ROOT_DIR})
target_include_directories(test_tasklists PRIVATE ${NEO_CLIENT_DIR}/include)
target_link_directories(test_tasklists PRIVATE ${NEO_CLIENT_DIR}/lib)
target_include_directories(test_tasklists PRIVATE ${EXTERNAL_DIR}/googletest/googletest/include)
target_include_directories(test_tasklists PRIVATE ${EXTERNAL_DIR}/googletest/googlemock/include)
target_link_libraries(test_tasklists PRIVATE tasklistsWorker DB.o gtest gmock neo4j-client)
