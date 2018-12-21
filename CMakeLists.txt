project(lib_mini_printf)


SET(LIB_MINI_PRINTF_SOURCE_C   		    "src/mini-printf.c")
SET(LIB_MINI_PRINTF_INTERFACE_HEADER    "include")

#######################################################################################
#Build target settings
#######################################################################################
add_library(${PROJECT_NAME} STATIC ${LIB_MINI_PRINTF_SOURCE_C})
target_include_directories(${PROJECT_NAME} PUBLIC ${LIB_MINI_PRINTF_INTERFACE_HEADER})
