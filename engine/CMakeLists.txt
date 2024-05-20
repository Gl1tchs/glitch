add_subdirectory(core)
add_subdirectory(renderer)
add_subdirectory(platform)

if (GL_BUILD_TESTS)
    add_subdirectory(tests)
endif()
