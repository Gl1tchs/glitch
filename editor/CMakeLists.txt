include(ShaderUtils)

file(GLOB_RECURSE COCKPIT_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
)

add_executable(glitch-editor ${COCKPIT_SOURCES})

target_include_directories(glitch-editor
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_SOURCE_DIR}/third_party/stb
)

target_link_libraries(glitch-editor PUBLIC glitch)

set(SHADER_LIST
    ${CMAKE_CURRENT_LIST_DIR}/shaders/infinite_grid.vert
    ${CMAKE_CURRENT_LIST_DIR}/shaders/infinite_grid.frag
)

compile_shader_group(COMPILED_SHADERS "${SHADER_LIST}" "")

add_custom_target(CompileShaders ALL
    DEPENDS ${COMPILED_SHADERS}
)

add_dependencies(glitch-editor CompileShaders)