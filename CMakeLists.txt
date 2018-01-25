cmake_minimum_required (VERSION 2.6)
project (minijson_test C)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()

add_library(minijson minijson.c)
add_executable(minijson_test test.c)
target_link_libraries(minijson_test minijson)