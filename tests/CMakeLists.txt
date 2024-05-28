set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests)

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(glitch-tests ${TEST_SOURCES})

target_include_directories(glitch-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CATCH2_INCLUDES}
)

target_link_libraries(glitch-tests PRIVATE
    glitch
    ${CATCH2_LIBRARIES}
)

list(APPEND CMAKE_MODULE_PATH ${CATCH2_SOURCE_DIR}/extras)

include(CTest)
include(Catch)
catch_discover_tests(glitch-tests)
