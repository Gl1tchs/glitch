cmake_minimum_required(VERSION 3.29)

project(glitch)

set(PROJECT_VERSION "0.1.0")

option(GL_INSTALL "Should target be installed" ON)
set(GL_INSTALL ${GL_INSTALL})

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

set(VENDOR_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party)

find_package(Vulkan REQUIRED)

add_subdirectory(third_party)

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/include/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

list(APPEND SOURCES ${IMGUI_FILES})
list(APPEND SOURCES ${SPIRV_REFLECT_FILES})

if (GL_BUILD_DYNAMIC_LIBS)
    add_library(glitch SHARED ${SOURCES})
else()
    add_library(glitch STATIC ${SOURCES})
endif()

target_include_directories(glitch
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${GLM_INCLUDES}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_BINARY_DIR}/include
    ${GLFW_INCLUDES}
    ${SPIRV_REFLECT_INCLUDES}
    ${VMA_INCLUDES}
    ${IMGUI_INCLUDES}
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(glitch
    PRIVATE
    ${GLFW_LIBS}
    ${VMA_LIBS}
    ${VK_BOOTSTRAP_LIBS}
    ${IMGUI_LIBS}
    ${Vulkan_LIBRARIES}
)

target_precompile_headers(glitch PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include/glitch/pch.h
)

target_compile_definitions(glitch PRIVATE
    GLFW_INCLUDE_NONE
    ${IMGUI_DEFINITIONS}
)

# Install target
if (GL_INSTALL)
    include(GNUInstallDirs)
    install(TARGETS glitch
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

    install(TARGETS glitch DESTINATION lib)

    install(
        DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
        DESTINATION include
    )

    # install glm
    install(
        DIRECTORY ${VENDOR_DIR}/glm/glm/
        DESTINATION include/glm
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.inl"
        PATTERN "CMakeLists.txt" EXCLUDE
    )

    # install imgui
    file(GLOB IMGUI_HEADERS "${VENDOR_DIR}/imgui/*.h")
    install(
        FILES ${IMGUI_HEADERS}
        DESTINATION include/imgui
    )

    # install stb
    install(
        DIRECTORY ${VENDOR_DIR}/stb/
        DESTINATION include/stb
    )  
endif()

if (GL_BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    # Configure Doxygen
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_MARKDOWN_SUPPORT YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_LIST_DIR}/README.md")
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_GENERATE_DISABLE_INDEX NO)
    set(DOXYGEN_GENERATE_FULL_SIDEBAR NO)
    set(DOXYGEN_HTML_COLORSTYLE LIGHT)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET
        "${CMAKE_CURRENT_LIST_DIR}/doc/doxygen/doxygen-awesome.css"
    )

    doxygen_add_docs(doxygen
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/README.md
    )
endif()

if (GL_BUILD_TESTS)
    add_subdirectory(tests)
endif()
