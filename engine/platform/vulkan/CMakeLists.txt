find_package(Vulkan REQUIRED)

file(GLOB_RECURSE VULKAN_SOURCES
    "${ENGINE_DIR}/platform/vulkan/*.cpp"
    "${ENGINE_DIR}/platform/vulkan/*.h"
)

add_library(gl-vulkan ${VULKAN_SOURCES})

target_include_directories(gl-vulkan PUBLIC
    ${ENGINE_DIR}
    # for the generated shader
    ${CMAKE_BINARY_DIR}/include
    ${VMA_INCLUDES}
    ${GLM_INCLUDES}
    ${VK_BOOTSTRAP_INCLUDES}
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(gl-vulkan PRIVATE
    gl-core
    gl-renderer
    ${VMA_LIBRARIES}
    ${GLM_LIBRARIES}
    ${VK_BOOTSTRAP_LIBRARIES}
    ${Vulkan_LIBRARIES}
)

target_precompile_headers(gl-vulkan PUBLIC ${ENGINE_DIR}/pch.h)
