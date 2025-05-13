file(GLOB_RECURSE TESTBED_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/examples/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/examples/*.h"
)

add_executable(glitch-testbed ${TESTBED_SOURCES})

target_include_directories(glitch-testbed
    PUBLIC
    src
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_SOURCE_DIR}/third_party/stb
)

target_link_libraries(glitch-testbed PUBLIC glitch)

target_precompile_headers(glitch-testbed PUBLIC src/pch.h)
