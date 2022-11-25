add_library(users OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/users.cpp)
target_include_directories(users PUBLIC ${ROOT_DIR})
