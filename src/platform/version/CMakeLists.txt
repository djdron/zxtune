project(libplatform_version)

file(GLOB SRCS_CXX "src/*.cpp" "text/text.cpp")
file(GLOB SRCS_H "*.h" "src/*.h" "text/text.h")

add_library(platform_version ${SRCS_CXX} ${SRCS_H})
