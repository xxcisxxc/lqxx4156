cmake_minimum_required(VERSION 3.5)

project(lqxx)

# pthread used by httplib (very out-dated...)
set(CMAKE_CXX_FLAGS -pthread)

# set third party dependences
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${PROJECT_SOURCE_DIR}/external/json)
add_subdirectory(${PROJECT_SOURCE_DIR}/external/googletest)
add_subdirectory(${PROJECT_SOURCE_DIR}/external/liboauthcpp/build)

# api library
add_library(api_library STATIC ${PROJECT_SOURCE_DIR}/src/api.cpp)
target_include_directories(api_library PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(api_library PUBLIC ${PROJECT_SOURCE_DIR}/external)
target_link_libraries(api_library PRIVATE nlohmann_json oauthcpp)

# api unit test
add_executable(test_api ${PROJECT_SOURCE_DIR}/tests/test_api.cpp)
target_link_libraries(test_api PRIVATE api_library gtest nlohmann_json)

# main executable
add_executable(lqxx ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(lqxx api_library)
