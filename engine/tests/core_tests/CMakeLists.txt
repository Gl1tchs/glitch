file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(gl-core-tests ${TEST_SOURCES})

target_include_directories(gl-core-tests PRIVATE
    ${ENGINE_DIR}
    ${CATCH2_INCLUDES}
)
target_link_libraries(gl-core-tests PRIVATE
    gl-core
    ${CATCH2_LIBRARIES}
)
