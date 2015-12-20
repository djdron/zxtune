project(libio)

file(GLOB SRCS_CXX "impl/*.cpp" "providers/enumerator.cpp" "providers/file_provider.cpp" "text/io.cpp")
file(GLOB SRCS_H "*.h" "impl/*.h" "providers/*.h" "text/io.h")

find_package(CURL)

if(CURL_FOUND)
list(APPEND SRCS_CXX "providers/gates/curl_api_dynamic.cpp" "providers/network_provider.cpp")
list(APPEND SRCS_H "providers/gates/curl_api.h")
else(CURL_FOUND)
list(APPEND SRCS_CXX "providers/network_stub.cpp")
endif(CURL_FOUND)

add_library(io ${SRCS_CXX} ${SRCS_H})
