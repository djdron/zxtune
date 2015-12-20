project(libplatform)

file(GLOB SRCS_CXX "src/*.cpp")
file(GLOB SRCS_H "*.h" "src/*.h")

if(WIN32)
list(APPEND SRCS_CXX "src/windows/shared_library.cpp" "src/windows/tools.cpp")
else(WIN32)
list(APPEND SRCS_CXX "src/linux/shared_library.cpp" "src/linux/tools.cpp")
endif(WIN32)

add_library(platform ${SRCS_CXX} ${SRCS_H})
set_property(TARGET platform PROPERTY FOLDER "platform")
