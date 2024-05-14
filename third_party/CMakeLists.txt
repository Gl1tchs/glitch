# define include dirs and library targets
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/src/third_party PARENT_SCOPE)

set(VMA_INCLUDES ${VENDOR_DIR}/VulkanMemoryAllocator/include PARENT_SCOPE)
set(VMA_LIBRARIES GPUOpen::VulkanMemoryAllocator PARENT_SCOPE)

set(FASTGLTF_INCLUDES ${VENDOR_DIR}/fastgltf/include PARENT_SCOPE)
set(FASTGLTF_LIBRARIES fastltf PARENT_SCOPE)

set(FMT_INCLUDES ${VENDOR_DIR}/fmt/include PARENT_SCOPE)
set(FMT_LIBRARIES fmt::fmt PARENT_SCOPE)

set(GLFW_INCLUDES ${VENDOR_DIR}/glfw/include PARENT_SCOPE)
set(GLFW_LIBRARIES glfw PARENT_SCOPE)

set(GLM_INCLUDES ${VENDOR_DIR}/glm PARENT_SCOPE)
set(GLM_LIBRARIES glm::glm-header-only PARENT_SCOPE)

set(IMGUI_DIR ${VENDOR_DIR}/imgui PARENT_SCOPE)
set(IMGUI_INCLUDES
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUI_DIR}/misc/cpp
    PARENT_SCOPE
)

set(STB_INCLUDES ${VENDOR_DIR}/stb/include PARENT_SCOPE)

set(VK_BOOTSTRAP_INCLUDES ${VENDOR_DIR}/vk-bootstrap/src PARENT_SCOPE)
set(VK_BOOTSTRAP_LIBRARIES vk-bootstrap::vk-bootstrap PARENT_SCOPE)

# define and build targets
set(VMA_BUILD_SAMPLES OFF CACHE BOOL "Build samples" FORCE)
add_subdirectory(VulkanMemoryAllocator)

add_subdirectory(fastgltf)

add_subdirectory(fmt)

add_subdirectory(glfw)

add_subdirectory(glm)

set(VK_BOOTSTRAP_TEST OFF CACHE BOOL "Test Vk-Bootstrap using Catch2 as well as build examples" FORCE)
add_subdirectory(vk-bootstrap)
