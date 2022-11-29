enable_testing()
add_definitions(--coverage)

include_directories(${ROOT_DIR})
link_libraries(neo4j-client gtest pthread gcov gmock)

add_executable(test_intg test_intg.cpp)
target_link_libraries(test_intg PRIVATE DB users tasklistsWorker tasksWorker)

include(GoogleTest)
gtest_discover_tests(test_intg)
