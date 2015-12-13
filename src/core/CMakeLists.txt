project(libcore)

file(GLOB SRCS_CXX "plugins/*.cpp" "src/*.cpp" "text/*.cpp")
file(GLOB SRCS_H "plugins/*.h" "src/*.h" "text/*.h" "*.h")

add_library(core ${SRCS_CXX} ${SRCS_H})
