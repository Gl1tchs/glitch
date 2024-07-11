cmake_minimum_required(VERSION 3.29)

project(glitch)

option(GL_BUILD_TESTS "Build tests!" ON)
set(GL_BUILD_TESTS ${GL_BUILD_TESTS})

option(GL_BUILD_TESTBED "Build testbed application!" ON)
set(GL_BUILD_TESTBED ${GL_BUILD_TESTBED})

# append into module path for our own configurations
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CMakeOptions)

add_subdirectory(third_party)

# build the engine
add_subdirectory(src)

if (GL_BUILD_TESTBED)
    add_subdirectory(testbed)
endif()

if (GL_BUILD_TESTS)
    add_subdirectory(tests)
endif()
