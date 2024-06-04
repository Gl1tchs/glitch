file(GLOB_RECURSE CORE_SOURCES "*.h" "*.cpp")

add_library(gl-core ${CORE_SOURCES})
add_library(gl::core ALIAS gl-core)

target_include_directories(gl-core
    PUBLIC
    ${ENGINE_DIR}
    ${GLM_INCLUDES}
    PRIVATE
    ${GLFW_INCLUDES}
)

target_link_libraries(gl-core
    PRIVATE
    gl::renderer
    PRIVATE
    ${GLFW_LIBS}
)

target_compile_definitions(gl-core PRIVATE GLFW_INCLUDE_NONE)

target_precompile_headers(gl-core PUBLIC ${ENGINE_DIR}/pch.h)
