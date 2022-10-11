cmake_minimum_required(VERSION 3.5)

project(lqxx)

set(CMAKE_CXX_FLAGS -pthread)

add_library(api_library STATIC ${PROJECT_SOURCE_DIR}/src/api.cpp)
target_include_directories(api_library PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(api_library PUBLIC ${PROJECT_SOURCE_DIR}/external)

add_executable(lqxx ${PROJECT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(lqxx api_library)