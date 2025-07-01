include(ShaderUtils)

set(PIPELINES_DIR ${CMAKE_CURRENT_LIST_DIR}/pipelines)
set(COMP_DIR ${CMAKE_CURRENT_LIST_DIR}/comp)
set(BUILD_DIR ${CMAKE_BINARY_DIR}/shaders)

file(GLOB_RECURSE VS_FILES ${PIPELINES_DIR}/*.vert)
file(GLOB_RECURSE FS_FILES ${PIPELINES_DIR}/*.frag)
file(GLOB_RECURSE COMP_FILES ${COMP_DIR}/*.comp)

set(SHADER_ROOT "${CMAKE_SOURCE_DIR}/shaders")

compile_shader_group(SPV_VS_LIST "${VS_FILES}" "${CMAKE_CURRENT_LIST_DIR}/include")
compile_shader_group(SPV_FS_LIST "${FS_FILES}" "${CMAKE_CURRENT_LIST_DIR}/include")
compile_shader_group(SPV_COMP_LIST "${COMP_FILES}" "${CMAKE_CURRENT_LIST_DIR}/include")

add_custom_target(gl-shaders ALL
    DEPENDS ${SPV_VS_LIST} ${SPV_FS_LIST} ${SPV_COMP_LIST}
)

# bundle the shaders if any provided
if (SPV_VS_LIST OR SPV_FS_LIST OR SPV_COMP_LIST)
    set(BUNDLER_EXE $<TARGET_FILE:gl-bundler>)

    set(BUNDLE_OUTPUT_DIR "${GL_OUTPUT_DIR}/include")

    add_custom_command(TARGET gl-shaders PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUNDLE_OUTPUT_DIR}
        COMMAND ${BUNDLER_EXE}
            ${BUNDLE_OUTPUT_DIR}/shader_bundle.gen.h
            ${BUILD_DIR}
            ${SPV_VS_LIST}
            ${SPV_FS_LIST}
            ${SPV_COMP_LIST}
    )
endif()

add_dependencies(gl-shaders gl-bundler)
