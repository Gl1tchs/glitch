cmake_minimum_required(VERSION 3.29)

project(glitch)

option(GL_BUILD_TESTS "Build tests!" ON)
set(GL_BUILD_TESTS ${GL_BUILD_TESTS})

option(GL_BUILD_DOCS "Build doxygen documentation" ON)
set(GL_BUILD_DOCS ${GL_BUILD_DOCS})

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

target_include_directories(glitch
    PUBLIC
    ${ENGINE_DIR}
    ${GLM_INCLUDES}
    PRIVATE
    ${CMAKE_BINARY_DIR}/include
    ${GLFW_INCLUDES}
    ${VMA_INCLUDES}
    ${IMGUI_INCLUDES}
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(glitch
    PRIVATE
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

if (GL_BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    # Configure Doxygen
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_EXCLUDE "${CMAKE_SOURCE_DIR}/src/platform")
    set(DOXYGEN_MARKDOWN_SUPPORT YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md")
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_GENERATE_DISABLE_INDEX NO)
    set(DOXYGEN_GENERATE_FULL_SIDEBAR NO)
    set(DOXYGEN_HTML_COLORSTYLE LIGHT)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET
        "${CMAKE_SOURCE_DIR}/doc/doxygen/doxygen-awesome.css"
    )

    doxygen_add_docs(doxygen
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/README.md
    )
endif()

if (GL_BUILD_TESTS)
    add_subdirectory(tests)
endif()
