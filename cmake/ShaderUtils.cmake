# Function: compile_slang_module
# Compiles a single Slang source file containing multiple entry points (e.g., vertexMain, fragmentMain)
# into one SPIR-V file.
# Arguments:
#   1 = Shader Source File (e.g., main.slang)
#   2 = Output File Path (e.g., build/shaders/main.spv)
#   3 = Output list variable name (to store the single output path)
#   4 = OPTIONAL Include Directory
function(compile_slang_module SHADER_FILE OUT_FILE OUT_LIST_VAR INCLUDE_DIR)
    if(NOT SLANGC)
        find_program(SLANGC slangc)
        if(NOT SLANGC)
            message(FATAL_ERROR "slangc not found! Please install the Slang SDK.")
        endif()
    endif()

    set(INCLUDE_FLAG "")
    if(INCLUDE_DIR)
        set(INCLUDE_FLAG -I "${INCLUDE_DIR}")
    endif()

    get_filename_component(OUT_DIR ${OUT_FILE} DIRECTORY)
    file(MAKE_DIRECTORY ${OUT_DIR})

    set(SLANG_FLAGS
        -target spirv
        -profile glsl_450
        -O2
        ${INCLUDE_FLAG}
    )

    add_custom_command(
        OUTPUT ${OUT_FILE}
        COMMAND ${SLANGC} ${SHADER_FILE} ${SLANG_FLAGS} -o ${OUT_FILE}
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling Slang Module: ${SHADER_FILE} -> ${OUT_FILE} (Single SPV)"
        VERBATIM
    )
    
    # Return the single SPV file to the parent scope
    set(${OUT_LIST_VAR} "${OUT_FILE}" PARENT_SCOPE)
endfunction()