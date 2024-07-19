cmake_minimum_required(VERSION 3.29)

project(glitch)

option(GL_BUILD_TESTS "Build tests!" ON)
set(GL_BUILD_TESTS ${GL_BUILD_TESTS})

option(GL_BUILD_TESTBED "Build testbed application!" ON)
set(GL_BUILD_TESTBED ${GL_BUILD_TESTBED})

option(GL_BUILD_DYNAMIC_LIBS "Build dynamic libraries" ON)
set(GL_BUILD_DYNAMIC_LIBS ${GL_BUILD_DYNAMIC_LIBS})

if (GL_BUILD_DYNAMIC_LIBS)
    add_compile_definitions(GL_EXPORT)
endif()

# append into module path for our own configurations
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CMakeOptions)

set(ENGINE_DIR ${CMAKE_SOURCE_DIR}/src)
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/third_party)

find_package(Vulkan REQUIRED)

add_subdirectory(third_party)
add_subdirectory(programs)
add_subdirectory(shaders)

file(GLOB_RECURSE SOURCES
    "${ENGINE_DIR}/*.h"
    "${ENGINE_DIR}/*.cpp"
)

list(APPEND SOURCES ${IMGUI_FILES})

if (GL_BUILD_DYNAMIC_LIBS)
    add_library(glitch SHARED ${SOURCES})
else()
    add_library(glitch STATIC ${SOURCES})
endif()

# make sure shaders has been builded before building the engine
add_dependencies(glitch gl-shaders)

target_include_directories(glitch
    PUBLIC
    ${ENGINE_DIR}
    ${GLM_INCLUDES}
    PRIVATE
    ${CMAKE_BINARY_DIR}/include
    ${GLFW_INCLUDES}
    ${VMA_INCLUDES}
    ${TINYGLTF_INCLUDES}
    ${IMGUI_INCLUDES}
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(glitch
    PRIVATE
    ${TINYGLTF_LIBS}
    ${SPIRV_REFLECT_LIBS}
    ${GLFW_LIBS}
    ${VMA_LIBS}
    ${VK_BOOTSTRAP_LIBS}
    ${IMGUI_LIBS}
    ${Vulkan_LIBRARIES}
)

target_precompile_headers(glitch PUBLIC ${ENGINE_DIR}/pch.h)

target_compile_definitions(glitch PRIVATE
    GLFW_INCLUDE_NONE
    ${IMGUI_DEFINITIONS}
)

if (GL_BUILD_TESTBED)
    add_subdirectory(testbed)
endif()

if (GL_BUILD_TESTS)
    add_subdirectory(tests)
endif()
