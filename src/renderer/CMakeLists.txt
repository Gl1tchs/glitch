file(GLOB_RECURSE RENDERER_SOURCES
    "${ENGINE_DIR}/renderer/*.h"
    "${ENGINE_DIR}/renderer/*.cpp"
)

add_library(gl-renderer ${RENDERER_SOURCES})
add_library(gl::renderer ALIAS gl-renderer)

target_include_directories(gl-renderer
    PUBLIC
    ${ENGINE_DIR}
    ${GLM_INCLUDES}
    PRIVATE
    ${CMAKE_BINARY_DIR}/include
    ${VMA_INCLUDES}
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(gl-renderer
    PRIVATE
    gl::vulkan-backend
)

target_precompile_headers(gl-renderer PUBLIC ${ENGINE_DIR}/pch.h)

# make sure shaders has been builded before building the engine
add_dependencies(gl-renderer gl-shaders)
