file(GLOB_RECURSE EDITOR_SOURCES
    "${EDITOR_DIR}/*.cpp"
    "${EDITOR_DIR}/*.h"
)

add_executable(gled ${EDITOR_SOURCES})

target_include_directories(gled PRIVATE
    ${ENGINE_DIR}
)

target_link_libraries(gled PRIVATE
    core    
)

target_precompile_headers(gled PUBLIC ${ENGINE_DIR}/pch.h)

