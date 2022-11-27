add_library(api OBJECT api.cpp ${EXTERNAL_DIR}/liboauthcpp/src/base64.cpp)
target_include_directories(api PUBLIC ${ROOT_DIR})
