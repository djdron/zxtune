project(libplatform_application)

file(GLOB SRCS_CXX "*.cpp")

add_library(platform_application ${SRCS_CXX})
set_property(TARGET platform_application PROPERTY FOLDER "platform")
