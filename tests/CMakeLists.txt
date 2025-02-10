find_package(doctest CONFIG REQUIRED)

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(glitch-tests ${TEST_SOURCES})

target_include_directories(glitch-tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(glitch-tests
    PRIVATE
    glitch
    PRIVATE
    doctest::doctest 
)
