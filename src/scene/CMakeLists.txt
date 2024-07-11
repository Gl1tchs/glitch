file(GLOB_RECURSE SCENE_SOURCES
    "${ENGINE_DIR}/scene/*.h"
    "${ENGINE_DIR}/scene/*.cpp"
)

add_library(gl-scene ${SCENE_SOURCES})
add_library(gl::scene ALIAS gl-scene)

target_include_directories(gl-scene
    PUBLIC
    ${ENGINE_DIR}
    ${GLM_INCLUDES}
)

target_link_libraries(gl-scene
    PRIVATE
    gl::core
)

target_precompile_headers(gl-scene PUBLIC ${ENGINE_DIR}/pch.h)
