file(READ ${FFI_LUA_FILE} FFI_LUA_CONTENT CONTENT)

configure_file(
    "${INPUT_FILE}"
    "${OUTPUT_FILE}"
    @ONLY
)