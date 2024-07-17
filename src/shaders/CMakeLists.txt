find_program(GLSLC glslc)
if(NOT GLSLC)
    message(FATAL_ERROR "glslc not found!\n"
            "Please make sure it is installed and available in your PATH.")
endif()

set(GLSL_FLAGS -I. --target-env=vulkan1.3 -std=450 -O -fpreserve-bindings)

set(SRC_DIR ${ENGINE_DIR}/shaders)
set(BUILD_DIR ${CMAKE_BINARY_DIR}/shaders)

file(GLOB_RECURSE VS_FILES ${SRC_DIR}/*.vert)
file(GLOB_RECURSE FS_FILES ${SRC_DIR}/*.frag)
file(GLOB_RECURSE COMP_FILES ${SRC_DIR}/*.comp)

add_custom_target(gl-shaders ALL)

add_custom_command(TARGET gl-shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_DIR}
)

foreach(VS_FILE ${VS_FILES})
    get_filename_component(VS_FILE_NAME ${VS_FILE} NAME)
    set(SPV_VS ${BUILD_DIR}/${VS_FILE_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPV_VS}
        COMMAND ${GLSLC} ${GLSL_FLAGS} -o ${SPV_VS} ${VS_FILE}
        DEPENDS ${VS_FILE}
    )
    list(APPEND SPV_VS_LIST ${SPV_VS})
endforeach()

foreach(FS_FILE ${FS_FILES})
    get_filename_component(FS_FILE_NAME ${FS_FILE} NAME)
    set(SPV_FS ${BUILD_DIR}/${FS_FILE_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPV_FS}
        COMMAND ${GLSLC} ${GLSL_FLAGS} -o ${SPV_FS} ${FS_FILE}
        DEPENDS ${FS_FILE}
    )
    list(APPEND SPV_FS_LIST ${SPV_FS})
endforeach()

foreach(COMP_FILE ${COMP_FILES})
    get_filename_component(COMP_FILE_NAME ${COMP_FILE} NAME)
    set(SPV_COMP ${BUILD_DIR}/${COMP_FILE_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPV_COMP}
        COMMAND ${GLSLC} ${GLSL_FLAGS} -o ${SPV_COMP} ${COMP_FILE}
        DEPENDS ${COMP_FILE}
    )
    list(APPEND SPV_COMP_LIST ${SPV_COMP})
endforeach()

add_custom_target(gl-shader-bundle ALL
    DEPENDS ${SPV_VS_LIST} ${SPV_FS_LIST} ${SPV_COMP_LIST}
)

# bundle the shaders if any provided
if (SPV_VS_LIST OR SPV_FS_LIST OR SPV_COMP_LIST)
    set(BUNDLER_EXE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/programs/gl-bundler)

    set(BUNDLE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/include")

    add_custom_command(TARGET gl-shader-bundle PRE_BUILD
        DEPENDS gl-bundler
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUNDLE_OUTPUT_DIR}
        COMMAND ${BUNDLER_EXE}
            ${BUNDLE_OUTPUT_DIR}/shader_bundle.gen.h
            ${SPV_VS_LIST}
            ${SPV_FS_LIST}
            ${SPV_COMP_LIST}
    )
endif()

add_dependencies(gl-shader-bundle gl-bundler)
