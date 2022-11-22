cmake_minimum_required(VERSION 3.0)

project(DBTest)

enable_testing()

# project top root directory
if (NOT DEFINED ROOT_DIR)
    set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif (NOT DEFINED ROOT_DIR)

add_library(DB.o OBJECT DB.cc)
target_include_directories(DB.o PRIVATE ${NEO_CLIENT_DIR}/include)
target_include_directories(DB.o PRIVATE ${ROOT_DIR})

add_executable(DBTest test/testDB.cc $<TARGET_OBJECTS:DB.o>)
target_include_directories(DBTest PRIVATE ${NEO_CLIENT_DIR}/include)
target_include_directories(DBTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(DBTest PRIVATE ${ROOT_DIR})
target_link_directories(DBTest PRIVATE ${NEO_CLIENT_DIR}/lib)
target_link_libraries(DBTest PRIVATE neo4j-client gtest pthread)

add_executable(testNeo testNeo.cc $<TARGET_OBJECTS:DB.o>)
target_include_directories(testNeo PRIVATE ${NEO_CLIENT_DIR}/include)
target_include_directories(testNeo PRIVATE ${ROOT_DIR})
target_link_directories(testNeo PRIVATE ${NEO_CLIENT_DIR}/lib)
target_link_libraries(testNeo PRIVATE neo4j-client pthread)

include(GoogleTest)
gtest_discover_tests(DBTest)