enable_testing()
add_definitions(--coverage)

include_directories(${ROOT_DIR})
link_libraries(neo4j-client gtest pthread gcov gmock)

add_executable(test_DB test_DB.cc ../db/DB.cc)

add_executable(test_tasklists test_tasklists.cpp ../tasklists/tasklistsWorker.cpp)
target_link_libraries(test_tasklists PRIVATE DB)

add_executable(test_tasks test_tasks.cpp ../tasks/tasksWorker.cpp)
target_link_libraries(test_tasks PRIVATE DB tasklistsWorker)

add_executable(test_users test_users.cpp ../users/users.cpp)
target_link_libraries(test_users PRIVATE DB)

add_executable(test_api test_api.cpp ../api/api.cpp ${EXTERNAL_DIR}/liboauthcpp/src/base64.cpp)
target_link_libraries(test_api PRIVATE DB users tasklistsWorker tasksWorker nlohmann_json ssl crypto)

include(GoogleTest)
gtest_discover_tests(test_DB)
gtest_discover_tests(test_tasklists)
gtest_discover_tests(test_tasks)
gtest_discover_tests(test_users)
gtest_discover_tests(test_api)