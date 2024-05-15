find_package(Vulkan REQUIRED)

file(GLOB_RECURSE RENDERER_SOURCES
    "${ENGINE_DIR}/renderer/*.cpp"
    "${ENGINE_DIR}/renderer/*.h"
)

add_library(gl-renderer ${RENDERER_SOURCES})

target_include_directories(gl-renderer PRIVATE
    ${ENGINE_DIR}
    ${GLM_INCLUDES}
    ${gl-vulkan_INCLUDE_DIRS}
)

target_link_libraries(gl-renderer PRIVATE
    gl-core
    gl-vulkan
)

target_precompile_headers(gl-renderer PUBLIC ${ENGINE_DIR}/pch.h)

