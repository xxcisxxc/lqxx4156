cmake_minimum_required(VERSION 3.0)

project(DBTest)

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