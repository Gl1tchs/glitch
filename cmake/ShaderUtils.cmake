# Function: compile_shader
# Arguments:
#   1 = Shader input file
#   2 = OPTIONAL include path
#   3 = OPTIONAL output file path (default: under BUILD_DIR with .spv extension)
function(compile_shader SHADER_FILE INCLUDE_DIR OUT_FILE)
    if(NOT GLSLC)
        find_program(GLSLC glslc)
        if(NOT GLSLC)
            message(FATAL_ERROR "glslc not found!\n"
                    "Please make sure it is installed and available in your PATH.")
        endif()
    endif()

    if(INCLUDE_DIR)
        set(INCLUDE_FLAG -I${INCLUDE_DIR})
    else()
        set(INCLUDE_FLAG "")
    endif()

    if(OUT_FILE)
        set(SPV_FILE ${OUT_FILE})
    else()
        file(RELATIVE_PATH SHADER_FILE_REL ${CMAKE_SOURCE_DIR} ${SHADER_FILE})
        set(SPV_FILE ${CMAKE_BINARY_DIR}/${SHADER_FILE_REL}.spv)
    endif()

    get_filename_component(SPV_FILE_DIR ${SPV_FILE} DIRECTORY)
    file(MAKE_DIRECTORY ${SPV_FILE_DIR})

    set(GLSL_FLAGS
        ${INCLUDE_FLAG}
        --target-env=vulkan1.3
        -std=450
        -O
        -fpreserve-bindings
    )

    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${GLSLC} ${GLSL_FLAGS} -o ${SPV_FILE} ${SHADER_FILE}
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling shader: ${SHADER_FILE} -> ${SPV_FILE}"
        VERBATIM
    )

    set(COMPILED_SHADER_OUT "${SPV_FILE}" PARENT_SCOPE)
endfunction()


# Function: compile_shader_group
# Arguments:
#   1 = output list variable (for SPV files)
#   3 = input list of shader source files
#   4 = OPTIONAL include directory (passed to GLSLC as -I)
function(compile_shader_group OUT_LIST SHADER_FILES INCLUDE_DIR)
    set(SPV_FILES "")  # Local list to collect SPV paths

    foreach(SHADER_FILE ${SHADER_FILES})
        file(RELATIVE_PATH SHADER_FILE_REL ${CMAKE_SOURCE_DIR} ${SHADER_FILE})
        set(OUT_FILE ${CMAKE_BINARY_DIR}/${SHADER_FILE_REL}.spv)

        # Call compile_shader with include path and custom output
        compile_shader("${SHADER_FILE}" "${INCLUDE_DIR}" "${OUT_FILE}")

        # Append compiled output
        list(APPEND SPV_FILES ${COMPILED_SHADER_OUT})
    endforeach()

    # Expose list to parent
    set(${OUT_LIST} "${SPV_FILES}" PARENT_SCOPE)
endfunction()
