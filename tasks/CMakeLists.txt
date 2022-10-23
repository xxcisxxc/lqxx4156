cmake_minimum_required(VERSION 3.5)

project(tasks)

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

# add tasklists, because mock need it
if (NOT TARGET tasklistsWorker)
    add_subdirectory(${ROOT_DIR}/tasklists ${ROOT_DIR}/build/tasklists)
endif()

# add neo4j
if (NOT DEFINED NEO_CLIENT_DIR)
    set(NEO_CLIENT_DIR /opt/homebrew/Cellar/libneo4j-client/2.2.0)
endif (NOT DEFINED NEO_CLIENT_DIR)

add_library(tasksWorker OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/tasksWorker.cpp)
target_include_directories(tasksWorker PRIVATE ${NEO_CLIENT_DIR}/include)
target_include_directories(tasksWorker PRIVATE ${ROOT_DIR})

add_executable(test_tasks ${CMAKE_CURRENT_SOURCE_DIR}/test/test_tasks.cpp)
target_include_directories(test_tasks PRIVATE ${ROOT_DIR})
target_include_directories(test_tasks PRIVATE ${ROOT_DIR}/tasklists)
target_include_directories(test_tasks PRIVATE ${NEO_CLIENT_DIR}/include)
target_link_directories(test_tasks PRIVATE ${NEO_CLIENT_DIR}/lib)
target_include_directories(test_tasks PRIVATE ${EXTERNAL_DIR}/googletest/googletest/include)
target_include_directories(test_tasks PRIVATE ${EXTERNAL_DIR}/googletest/googlemock/include)
target_link_libraries(test_tasks PRIVATE tasksWorker tasklistsWorker DB.o gtest gmock neo4j-client)

