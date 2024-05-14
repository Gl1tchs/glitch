cmake_minimum_required(VERSION 3.29)

project(glitch)

# append into module path for our own configurations
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CMakeOptions)

set(ENGINE_DIR ${CMAKE_SOURCE_DIR}/engine)
set(EDITOR_DIR ${CMAKE_SOURCE_DIR}/editor)

# needs to build first because of the variables
add_subdirectory(third_party)

add_subdirectory(editor)
add_subdirectory(engine)
add_subdirectory(programs)
add_subdirectory(shaders)
