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

# STB
set(STB_INCLUDES ${VENDOR_DIR}/stb PARENT_SCOPE)
