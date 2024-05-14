find_package(Vulkan REQUIRED)

file(GLOB_RECURSE RENDERER_SOURCES
    "${ENGINE_DIR}/renderer/*.cpp"
    "${ENGINE_DIR}/renderer/*.h"
)

add_library(renderer ${RENDERER_SOURCES})

target_include_directories(renderer PRIVATE
    ${ENGINE_DIR}
    ${VMA_INCLUDES}
    ${GLM_INCLUDES}
    ${VK_BOOTSTRAP_INCLUDES}
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(renderer PRIVATE
    core
    ${VMA_LIBRARIES}
    ${GLM_LIBRARIES}
    ${VK_BOOTSTRAP_LIBRARIES}
    ${Vulkan_LIBRARIES}
)

target_precompile_headers(renderer PUBLIC ${ENGINE_DIR}/pch.h)

