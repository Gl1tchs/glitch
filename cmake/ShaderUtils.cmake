find_program(GLSLC glslc)
if(NOT GLSLC)
    message(FATAL_ERROR "glslc not found!\n"
            "Please make sure it is installed and available in your PATH.")
endif()

# Function: compile_shader_group
# Arguments:
#   1 = output list variable (for SPV files)
#   2 = shader root folder (e.g. ${CMAKE_SOURCE_DIR}/shaders)
#   3 = input list of shader source files
#   4 = OPTIONAL include directory (passed to GLSLC as -I)
function(compile_shader_group OUT_LIST SHADER_ROOT_FOLDER SHADER_FILES INCLUDE_DIR)
    if(INCLUDE_DIR)
        set(INCLUDE_FLAG -I${INCLUDE_DIR})
    else()
        set(INCLUDE_FLAG "")
    endif()

    set(GLSL_FLAGS
        ${INCLUDE_FLAG}
        --target-env=vulkan1.3
        -std=450
        -O
        -fpreserve-bindings
    )

    foreach(SHADER_FILE ${SHADER_FILES})
        # Compute relative path from shader root folder
        file(RELATIVE_PATH SHADER_FILE_REL ${SHADER_ROOT_FOLDER} ${SHADER_FILE})

        # Build output path: BUILD_DIR/relative/path/to/shader.spv
        set(SPV_FILE ${BUILD_DIR}/${SHADER_FILE_REL}.spv)

        # Ensure output directory exists
        get_filename_component(SPV_FILE_DIR ${SPV_FILE} DIRECTORY)
        file(MAKE_DIRECTORY ${SPV_FILE_DIR})

        # Create compile command
        add_custom_command(
            OUTPUT ${SPV_FILE}
            COMMAND ${GLSLC} ${GLSL_FLAGS} -o ${SPV_FILE} ${SHADER_FILE}
            DEPENDS ${SHADER_FILE}
        )

        # Append to output list
        list(APPEND ${OUT_LIST} ${SPV_FILE})
    endforeach()

    # Set the output list as a PARENT_SCOPE variable (so it's usable outside this function)
    set(${OUT_LIST} "${${OUT_LIST}}" PARENT_SCOPE)
endfunction()