cmake_minimum_required(VERSION 3.0)

project(DBTest)
find_package(GTest CONFIG REQUIRED)
find_package (mongoc-1.0 1.7 REQUIRED)

add_library(DB.o OBJECT DB.cc)
target_link_libraries(DB.o PRIVATE mongo::mongoc_static)
add_executable(DBTest test/testDB.cc $<TARGET_OBJECTS:DB.o>)
target_include_directories(DBTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DBTest PRIVATE GTest::gtest_main mongo::mongoc_static)