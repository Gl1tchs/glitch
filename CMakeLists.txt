cmake_minimum_required(VERSION 3.29)

project(glitch)

option(GL_BUILD_TESTS "Build tests!" ON)
set(GL_BUILD_TESTS ${GL_BUILD_TESTS})

option(GL_BUILD_TESTBED "Build testbed application!" ON)
set(GL_BUILD_TESTBED ${GL_BUILD_TESTBED})

# append into module path for our own configurations
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CMakeOptions)

find_package(Vulkan REQUIRED)

add_subdirectory(third_party)
add_subdirectory(programs)
add_subdirectory(shaders)

file(GLOB_RECURSE SOURCES
    "include/*.h"
    "src/*.h"
    "src/*.cpp"
)

add_library(glitch ${SOURCES})

# make sure shaders has been builded before building the engine
add_dependencies(glitch shaders)

target_include_directories(glitch PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${GLM_INCLUDES}
)

target_include_directories(glitch PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/include
    ${TINYGLTF_INCLUDES}
    ${GLFW_INCLUDES}
    ${STB_INCLUDES}
    ${VMA_INCLUDES}
    ${VK_BOOTSTRAP_INCLUDES}
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(glitch PUBLIC 
    ${GLM_LIBRARIES}
)

target_link_libraries(glitch PRIVATE
    ${TINYGLTF_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${VMA_LIBRARIES}
    ${VK_BOOTSTRAP_LIBRARIES}
    ${Vulkan_LIBRARIES}
)

target_precompile_headers(glitch PUBLIC include/pch.h)

if (GL_BUILD_TESTBED)
    add_subdirectory(testbed)
endif()

if (GL_BUILD_TESTS)
    add_subdirectory(tests)
endif()
