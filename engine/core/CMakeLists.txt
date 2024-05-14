file(GLOB_RECURSE CORE_SOURCES
    "${ENGINE_DIR}/core/*.cpp"
    "${ENGINE_DIR}/core/*.h"
)

add_library(core ${CORE_SOURCES})

target_include_directories(core PRIVATE
    ${ENGINE_DIR}
    ${GLFW_INCLUDES}
    ${FMT_INCLUDES}
)

target_link_libraries(core PRIVATE
    renderer
    ${GLFW_LIBRARIES}
    ${FMT_LIBRARIES}
)

target_precompile_headers(core PUBLIC ${ENGINE_DIR}/pch.h)

