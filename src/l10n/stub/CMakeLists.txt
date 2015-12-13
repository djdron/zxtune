project(libl10n_stub)

file(GLOB SRCS_CXX "*.cpp")
file(GLOB SRCS_H "*.h" "../*.h" "../src/*.h")

add_library(l10n_stub ${SRCS_CXX} ${SRCS_H})
