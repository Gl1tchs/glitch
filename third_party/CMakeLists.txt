# define include dirs and library targets
set(VENDOR_DIR ${CMAKE_CURRENT_LIST_DIR})

# doctest
set(DOCTEST_INCLUDES ${VENDOR_DIR}/doctest/doctest PARENT_SCOPE)
set(DOCTEST_LIBS doctest::doctest PARENT_SCOPE)

if (GL_BUILD_TESTS)
    add_subdirectory(doctest)
endif()

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

set(IMGUI_USE_DARK_THEME ON CACHE BOOL "" FORCE)
set(IMGUI_DEFINE_MATH_OPERATORS ON CACHE BOOL "" FORCE)
add_subdirectory(imgui)

# SPIRV-Reflect
set(SPIRV_REFLECT_INCLUDES ${VENDOR_DIR}/spirv-reflect PARENT_SCOPE)
set(SPIRV_REFLECT_LIBS spirv-reflect-static PARENT_SCOPE)

set(SPIRV_REFLECT_EXECUTABLE OFF CACHE INTERNAL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE INTERNAL "" FORCE)
add_subdirectory(spirv-reflect)

# STB
set(STB_INCLUDES ${VENDOR_DIR}/stb/include PARENT_SCOPE)

# TinyGLTF
set(TINYGLTF_INCLUDES ${VENDOR_DIR}/tinygltf PARENT_SCOPE)
set(TINYGLTF_LIBS tinygltf PARENT_SCOPE)

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory(tinygltf)

# VkBootstrap
set(VK_BOOTSTRAP_INCLUDES ${VENDOR_DIR}/vk-bootstrap/src PARENT_SCOPE)
set(VK_BOOTSTRAP_LIBS vk-bootstrap::vk-bootstrap PARENT_SCOPE)

set(VK_BOOTSTRAP_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(vk-bootstrap)

# VulkanMemoryAllocator
set(VMA_INCLUDES ${VENDOR_DIR}/vulkan-memory-allocator/include PARENT_SCOPE)
set(VMA_LIBS GPUOpen::VulkanMemoryAllocator PARENT_SCOPE)

set(VMA_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vulkan-memory-allocator)
