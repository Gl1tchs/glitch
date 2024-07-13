set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests)

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(glitch-tests ${TEST_SOURCES})

target_include_directories(glitch-tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${DOCTEST_INCLUDES}
)

target_link_libraries(glitch-tests
    PRIVATE
    gl::core
    gl::renderer
    gl::scene
    PRIVATE
    ${DOCTEST_LIBS}
)
