enable_testing()

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

# Create test executable
add_executable(glitch-doctests ${TEST_SOURCES})

target_include_directories(glitch-doctests
	PRIVATE
	${CMAKE_SOURCE_DIR}/engine
	${DOCTEST_INCLUDES}
)

target_link_libraries(glitch-doctests
	PRIVATE
	glitch
	${DOCTEST_LIBS}
)

add_test(NAME Doctests COMMAND glitch-doctests)
