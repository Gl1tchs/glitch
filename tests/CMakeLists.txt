include(FetchContent)

find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found locally. Fetching v3.4.0 from GitHub...")
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)
endif()

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

# Create test executable
add_executable(glitch_tests ${TEST_SOURCES})

target_include_directories(glitch_tests
	PRIVATE
	${CMAKE_SOURCE_DIR}/engine
	${DOCTEST_INCLUDES}
)

target_link_libraries(glitch_tests
	PRIVATE
	glitch
    Catch2::Catch2WithMain
)
