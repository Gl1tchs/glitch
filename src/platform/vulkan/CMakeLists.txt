find_package(Vulkan REQUIRED)

file(GLOB_RECURSE VULKAN_BACKEND_SOURCES "*.h" "*.cpp")

add_library(gl-vulkan-backend ${VULKAN_BACKEND_SOURCES})
add_library(gl::vulkan-backend ALIAS gl-vulkan-backend)

target_include_directories(gl-vulkan-backend
    PUBLIC
    ${ENGINE_DIR}
    ${GLM_INCLUDES}
    ${VMA_INCLUDES}
    ${Vulkan_INCLUDE_DIRS}
    PRIVATE
    ${SPIRV_REFLECT_INCLUDES}
    ${GLFW_INCLUDES}
    ${VK_BOOTSTRAP_INCLUDES}
    ${IMGUI_INCLUDES}
)

target_link_libraries(gl-vulkan-backend
    PRIVATE
    gl::core
    gl::renderer
    PRIVATE
    ${SPIRV_REFLECT_LIBS}
    ${GLFW_LIBS}
    ${VMA_LIBS}
    ${VK_BOOTSTRAP_LIBS}
    ${Vulkan_LIBRARIES}
    ${IMGUI_LIBS}
)

target_precompile_headers(gl-vulkan-backend PUBLIC ${ENGINE_DIR}/pch.h)
