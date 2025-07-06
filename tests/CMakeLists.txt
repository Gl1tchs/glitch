file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(doctests ${TEST_SOURCES})

target_include_directories(doctests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/engine
    ${DOCTEST_INCLUDES}
)

target_link_libraries(doctests
    PRIVATE
    glitch
    PRIVATE
    ${DOCTEST_LIBS}
)
