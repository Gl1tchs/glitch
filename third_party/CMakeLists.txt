# doctest
set(DOCTEST_INCLUDES ${VENDOR_DIR}/doctest/doctest PARENT_SCOPE)
set(DOCTEST_LIBS doctest::doctest PARENT_SCOPE)

if (GL_BUILD_TESTS)
    set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
    add_subdirectory(doctest)
endif()

# Tracy profiler
set(TRACY_INCLUDES ${VENDOR_DIR}/tracy/public PARENT_SCOPE)
set(TRACY_LIBS TracyClient PARENT_SCOPE)

if (GL_ENABLE_PROFILING)
    add_subdirectory(tracy)
endif()

# GLFW
set(GLFW_INCLUDES ${VENDOR_DIR}/glfw/include PARENT_SCOPE)
set(GLFW_LIBS glfw PARENT_SCOPE)

set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# glm
set(GLM_INCLUDES ${VENDOR_DIR}/glm PARENT_SCOPE)

# ImGui
set(IMGUI_INCLUDES ${VENDOR_DIR}/imgui PARENT_SCOPE)
set(IMGUI_FILES
    "${VENDOR_DIR}/imgui/imgui.h"
    "${VENDOR_DIR}/imgui/imgui_internal.h"
    "${VENDOR_DIR}/imgui/imgui.cpp"
    "${VENDOR_DIR}/imgui/imgui_demo.cpp"
    "${VENDOR_DIR}/imgui/imgui_draw.cpp"
    "${VENDOR_DIR}/imgui/imgui_widgets.cpp"
    "${VENDOR_DIR}/imgui/imgui_tables.cpp"
    "${VENDOR_DIR}/imgui/backends/imgui_impl_vulkan.h"
    "${VENDOR_DIR}/imgui/backends/imgui_impl_vulkan.cpp"
    "${VENDOR_DIR}/imgui/backends/imgui_impl_glfw.h"
    "${VENDOR_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    "${VENDOR_DIR}/imgui/misc/cpp/imgui_stdlib.h"
    "${VENDOR_DIR}/imgui/misc/cpp/imgui_stdlib.cpp"
    PARENT_SCOPE
)

# SPIRV-Reflect
set(SPIRV_REFLECT_INCLUDES ${VENDOR_DIR}/spirv-reflect PARENT_SCOPE)
set(SPIRV_REFLECT_FILES
    "${VENDOR_DIR}/spirv-reflect/spirv_reflect.h"
    "${VENDOR_DIR}/spirv-reflect/spirv_reflect.cpp"
    PARENT_SCOPE
)

# STB
set(STB_INCLUDES ${VENDOR_DIR}/stb PARENT_SCOPE)

# VkBootstrap
set(VK_BOOTSTRAP_INCLUDES ${VENDOR_DIR}/vk-bootstrap/src PARENT_SCOPE)
set(VK_BOOTSTRAP_LIBS vk-bootstrap::vk-bootstrap PARENT_SCOPE)

set(VK_BOOTSTRAP_TEST OFF CACHE BOOL "" FORCE)
set(VK_BOOTSTRAP_INSTALL OFF CACHE BOOL "" FORCE)
set(VK_BOOTSTRAP_POSITION_INDEPENDENT_CODE ${GL_BUILD_DYNAMIC_LIBS} CACHE BOOL "" FORCE)
add_subdirectory(vk-bootstrap)

# VulkanMemoryAllocator
set(VMA_INCLUDES ${VENDOR_DIR}/VulkanMemoryAllocator/include PARENT_SCOPE)
set(VMA_LIBS GPUOpen::VulkanMemoryAllocator PARENT_SCOPE)

set(VMA_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(VulkanMemoryAllocator)