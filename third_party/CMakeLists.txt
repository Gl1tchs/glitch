# define include dirs and library targets
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/third_party)

# TinyGLTF
set(TINYGLTF_INCLUDES ${VENDOR_DIR}/tinygltf PARENT_SCOPE)
set(TINYGLTF_LIBS tinygltf PARENT_SCOPE)

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory(tinygltf)

# Catch2
set(CATCH2_SOURCE_DIR ${VENDOR_DIR}/catch2 PARENT_SCOPE)
set(CATCH2_INCLUDES ${VENDOR_DIR}/catch2/src PARENT_SCOPE)
set(CATCH2_LIBS Catch2::Catch2WithMain PARENT_SCOPE)

if (GL_BUILD_TESTS)
    add_subdirectory(catch2)
endif()

# SPIRV-Reflect
set(SPIRV_REFLECT_INCLUDES ${VENDOR_DIR}/spirv-reflect PARENT_SCOPE)
set(SPIRV_REFLECT_LIBS spirv-reflect-static PARENT_SCOPE)

set(SPIRV_REFLECT_EXECUTABLE OFF CACHE INTERNAL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE INTERNAL "" FORCE)
add_subdirectory(spirv-reflect)

# VulkanMemoryAllocator
set(VMA_INCLUDES ${VENDOR_DIR}/vulkan-memory-allocator/include PARENT_SCOPE)
set(VMA_LIBS GPUOpen::VulkanMemoryAllocator PARENT_SCOPE)

set(VMA_BUILD_SAMPLES OFF CACHE BOOL "Build samples" FORCE)
add_subdirectory(vulkan-memory-allocator)

# GLFW
set(GLFW_INCLUDES ${VENDOR_DIR}/glfw/include PARENT_SCOPE)
set(GLFW_LIBS glfw PARENT_SCOPE)

add_subdirectory(glfw)

# glm
set(GLM_INCLUDES ${VENDOR_DIR}/glm PARENT_SCOPE)
set(GLM_LIBS glm::glm PARENT_SCOPE)

add_subdirectory(glm)

# ImGui
set(IMGUI_DIR ${VENDOR_DIR}/imgui PARENT_SCOPE)
set(IMGUI_LIBS imgui PARENT_SCOPE)

add_subdirectory(imgui)

# STB
set(STB_INCLUDES ${VENDOR_DIR}/stb/include PARENT_SCOPE)

# VkBootstrap
set(VK_BOOTSTRAP_INCLUDES ${VENDOR_DIR}/vk-bootstrap/src PARENT_SCOPE)
set(VK_BOOTSTRAP_LIBS vk-bootstrap::vk-bootstrap PARENT_SCOPE)

set(VK_BOOTSTRAP_TEST OFF CACHE BOOL "Test Vk-Bootstrap using Catch2 as well as build examples" FORCE)
add_subdirectory(vk-bootstrap)
