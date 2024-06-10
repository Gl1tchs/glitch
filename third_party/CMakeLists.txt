# define include dirs and library targets
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/third_party)

set(TINYGLTF_INCLUDES ${VENDOR_DIR}/tinygltf PARENT_SCOPE)
set(TINYGLTF_LIBS tinygltf PARENT_SCOPE)

set(CATCH2_SOURCE_DIR ${VENDOR_DIR}/Catch2 PARENT_SCOPE)
set(CATCH2_INCLUDES ${VENDOR_DIR}/Catch2/src PARENT_SCOPE)
set(CATCH2_LIBS Catch2::Catch2WithMain PARENT_SCOPE)

set(SPIRV_REFLECT_INCLUDES ${VENDOR_DIR}/SPIRV-Reflect PARENT_SCOPE)
set(SPIRV_REFLECT_LIBS spirv-reflect-static PARENT_SCOPE)

set(VMA_INCLUDES ${VENDOR_DIR}/VulkanMemoryAllocator/include PARENT_SCOPE)
set(VMA_LIBS GPUOpen::VulkanMemoryAllocator PARENT_SCOPE)

set(GLFW_INCLUDES ${VENDOR_DIR}/glfw/include PARENT_SCOPE)
set(GLFW_LIBS glfw PARENT_SCOPE)

set(GLM_INCLUDES ${VENDOR_DIR}/glm PARENT_SCOPE)
set(GLM_LIBS glm::glm PARENT_SCOPE)

set(IMGUI_DIR ${VENDOR_DIR}/imgui PARENT_SCOPE)
set(IMGUI_LIBS imgui PARENT_SCOPE)

set(STB_INCLUDES ${VENDOR_DIR}/stb/include PARENT_SCOPE)

set(VK_BOOTSTRAP_INCLUDES ${VENDOR_DIR}/vk-bootstrap/src PARENT_SCOPE)
set(VK_BOOTSTRAP_LIBS vk-bootstrap::vk-bootstrap PARENT_SCOPE)

if (GL_BUILD_TESTS)
    add_subdirectory(Catch2)
endif()

set(SPIRV_REFLECT_EXECUTABLE OFF CACHE INTERNAL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE INTERNAL "" FORCE)
add_subdirectory(SPIRV-Reflect)

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory(tinygltf)

# define and build targets
set(VMA_BUILD_SAMPLES OFF CACHE BOOL "Build samples" FORCE)
add_subdirectory(VulkanMemoryAllocator)

add_subdirectory(glfw)

add_subdirectory(glm)

add_subdirectory(imgui)

set(VK_BOOTSTRAP_TEST OFF CACHE BOOL "Test Vk-Bootstrap using Catch2 as well as build examples" FORCE)
add_subdirectory(vk-bootstrap)
