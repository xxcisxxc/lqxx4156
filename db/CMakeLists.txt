cmake_minimum_required(VERSION 3.0)

project(DBTest)

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

add_library(DB.o OBJECT DB.cc)
target_include_directories(DB.o PRIVATE ${NEO_CLIENT_DIR}/include)

add_executable(DBTest test/testDB.cc $<TARGET_OBJECTS:DB.o>)
target_include_directories(DBTest PRIVATE ${NEO_CLIENT_DIR}/include)
target_include_directories(DBTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(DBTest PRIVATE ${NEO_CLIENT_DIR}/lib)
target_link_libraries(DBTest PRIVATE neo4j-client gtest)

add_executable(testNeo testNeo.cc $<TARGET_OBJECTS:DB.o>)
target_include_directories(testNeo PRIVATE ${NEO_CLIENT_DIR}/include)
target_link_directories(testNeo PRIVATE ${NEO_CLIENT_DIR}/lib)
target_link_libraries(testNeo PRIVATE neo4j-client pthread)