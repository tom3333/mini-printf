project(lib_mini_printf)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "mini-printf.c")


#Generate the static library from the sources

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ./)
