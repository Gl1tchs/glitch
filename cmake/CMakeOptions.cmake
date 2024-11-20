# Set C++ standard and additional flags as needed
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Disable nullable warnings for Clang
    add_compile_options(-Wno-nullability-completeness)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(GL_DEBUG_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(GL_RELEASE_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(GL_DEBUG_BUILD)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    add_compile_definitions(GL_RELEASE_BUILD)
    add_compile_definitions(GL_DIST_BUILD)
endif()
