file(GLOB_RECURSE TESTBED_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
)

add_executable(glitch-testbed ${TESTBED_SOURCES})

target_include_directories(glitch-testbed
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${STB_INCLUDES}
    ${IMGUI_INCLUDES}
)

target_link_libraries(glitch-testbed
    PRIVATE
    glitch
    PRIVATE
    ${IMGUI_LIBS}
)

target_precompile_headers(glitch-testbed
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src/pch.h
)

