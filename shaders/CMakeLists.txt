include(ShaderUtils)

set(PIPELINES_DIR ${CMAKE_CURRENT_LIST_DIR}/pipelines)
set(COMP_DIR ${CMAKE_CURRENT_LIST_DIR}/comp)
set(BUILD_DIR ${CMAKE_BINARY_DIR}/shaders)

set(SLANG_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")

file(GLOB_RECURSE PIPELINE_SLANG_FILES ${PIPELINES_DIR}/*.slang)
file(GLOB_RECURSE COMP_SLANG_FILES ${COMP_DIR}/*.slang)

set(ALL_SPV_FILES "")

foreach(SLANG_FILE ${PIPELINE_SLANG_FILES})
    file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_LIST_DIR} ${SLANG_FILE})
    get_filename_component(BASE_NAME ${REL_PATH} NAME_WE)
    set(OUT_SPV_FILE "${BUILD_DIR}/${REL_PATH}.spv")
    
    set(SPV_LIST "")
    compile_slang_module(
        "${SLANG_FILE}" 
        "${OUT_SPV_FILE}"
        SPV_LIST 
        "${SLANG_INCLUDE_DIR}"
    )
    list(APPEND ALL_SPV_FILES ${SPV_LIST})
endforeach()

foreach(SLANG_FILE ${COMP_SLANG_FILES})
    file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_LIST_DIR} ${SLANG_FILE})
    set(OUT_SPV_FILE "${BUILD_DIR}/${REL_PATH}.spv")

    set(SPV_FILE "")
    compile_slang_module(
        "${SLANG_FILE}" 
        "${OUT_SPV_FILE}"
        SPV_FILE
        "${SLANG_INCLUDE_DIR}"
    )
    list(APPEND ALL_SPV_FILES "${SPV_FILE}")
endforeach()

add_custom_target(slang-shaders ALL
    DEPENDS ${ALL_SPV_FILES}
)

if (ALL_SPV_FILES)
    set(BUNDLER_EXE $<TARGET_FILE:gl-bundler>)
    set(BUNDLE_OUTPUT_DIR "${GL_OUTPUT_DIR}/include")

    add_custom_command(TARGET slang-shaders PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUNDLE_OUTPUT_DIR}
        COMMAND ${BUNDLER_EXE}
            ${BUNDLE_OUTPUT_DIR}/shader_bundle.gen.h
            ${BUILD_DIR}
            ${ALL_SPV_FILES}
    )
endif()

add_dependencies(slang-shaders gl-bundler)