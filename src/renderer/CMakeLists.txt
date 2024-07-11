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
    ${TINYGLTF_INCLUDES}
    ${IMGUI_INCLUDES}
    ${GLFW_INCLUDES}
)

target_link_libraries(gl-renderer
    PRIVATE
    gl::scene
    gl::vulkan-backend
    PRIVATE
    ${TINYGLTF_LIBS}
    ${IMGUI_LIBS}
    ${GLFW_LIBS}
)

target_precompile_headers(gl-renderer PUBLIC ${ENGINE_DIR}/pch.h)

target_compile_definitions(gl-renderer PRIVATE GLFW_INCLUDE_NONE)

# make sure shaders has been builded before building the engine
add_dependencies(gl-renderer gl-shaders)
