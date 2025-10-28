cmake_minimum_required(VERSION 3.20)
project(lua
    VERSION 5.4.8
    LANGUAGES C
)

file(GLOB_RECURSE LUA_SOURCES
	"${CMAKE_CURRENT_LIST_DIR}/src/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/src/*.c"
)

add_library(lua STATIC ${LUA_SOURCES})
