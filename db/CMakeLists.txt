cmake_minimum_required(VERSION 3.0)

project(DBTest)
find_package(GTest CONFIG REQUIRED)

add_library(DB.o OBJECT DB.cc)
add_executable(DBTest test/testDB.cc $<TARGET_OBJECTS:DB.o>)
target_include_directories(DBTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DBTest GTest::gtest_main)