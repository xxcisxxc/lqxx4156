enable_testing()
add_definitions(--coverage)

include_directories(${ROOT_DIR})
link_libraries(neo4j-client gtest pthread gcov)

add_executable(test_system test_system.cpp ${ROOT_DIR}/api/api.cpp ${EXTERNAL_DIR}/liboauthcpp/src/base64.cpp ${ROOT_DIR}/db/DB.cc ${ROOT_DIR}/users/users.cpp ${ROOT_DIR}/tasklists/tasklistsWorker.cpp ${ROOT_DIR}/tasks/tasksWorker.cpp)
target_link_libraries(test_system PRIVATE nlohmann_json ssl crypto)

include(GoogleTest)
gtest_discover_tests(test_system)
