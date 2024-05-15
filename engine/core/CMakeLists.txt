file(GLOB_RECURSE CORE_SOURCES
    "${ENGINE_DIR}/core/*.cpp"
    "${ENGINE_DIR}/core/*.h"
)

add_library(gl-core ${CORE_SOURCES})

target_include_directories(gl-core PRIVATE
    ${ENGINE_DIR}
    ${GLFW_INCLUDES}
    ${FMT_INCLUDES}
)

target_link_libraries(gl-core PRIVATE
    gl-renderer
    ${GLFW_LIBRARIES}
    ${FMT_LIBRARIES}
)

target_precompile_headers(gl-core PUBLIC ${ENGINE_DIR}/pch.h)

