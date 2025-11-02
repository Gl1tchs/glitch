# Set C++ standard and additional flags as needed
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Disable nullable warnings for Clang
    add_compile_options(-Wno-nullability-completeness -Wno-c99-designator -Wno-deprecated-literal-operator)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GL_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin/debug)
    add_compile_definitions(GL_DEBUG_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(GL_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin/release)
    add_compile_definitions(GL_RELEASE_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(GL_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin/relwithdebinfo)
    add_compile_definitions(GL_DEBUG_BUILD)
    add_compile_definitions(GL_RELEASE_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(GL_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin/dist)
    add_compile_definitions(GL_RELEASE_BUILD)
    add_compile_definitions(GL_DIST_BUILD)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GL_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GL_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GL_OUTPUT_DIR})
