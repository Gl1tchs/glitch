include(ShaderUtils)

file(GLOB_RECURSE EDITOR_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
)

add_executable(glitch-editor ${EDITOR_SOURCES})

target_include_directories(glitch-editor
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_SOURCE_DIR}/engine
    ${IMGUI_INCLUDES}
    ${GLM_INCLUDES}
    ${CMAKE_SOURCE_DIR}/third_party
)

target_link_libraries(glitch-editor PRIVATE
    glitch
    ${TINYFD_LIBS}
)

set(SLANG_FILE "${CMAKE_CURRENT_LIST_DIR}/shaders/infinite_grid.slang")
file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_LIST_DIR} ${SLANG_FILE})
get_filename_component(BASE_NAME ${REL_PATH} NAME_WE)
set(OUT_SPV_FILE "${BUILD_DIR}/${REL_PATH}.spv")

set(SPV_LIST "")
compile_slang_module(
    "${SLANG_FILE}"
    "${OUT_SPV_FILE}"
    SPV_LIST 
    "${SLANG_INCLUDE_DIR}"
)

add_custom_target(CompileShaders ALL
    DEPENDS ${COMPILED_SHADERS}
)

add_dependencies(glitch-editor CompileShaders)