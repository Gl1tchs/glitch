cmake_minimum_required(VERSION 3.28)

project(glitch)

set(PROJECT_VERSION "0.1.0")

option(GL_BUILD_EDITOR "Build editor application" ON)
set(GL_BUILD_EDITOR ${GL_BUILD_EDITOR})

option(GL_BUILD_TESTS "Build tests!" ON)
set(GL_BUILD_TESTS ${GL_BUILD_TESTS})

option(GL_ENABLE_PROFILING "Enable profiling" ON)
set(GL_ENABLE_PROFILING ${GL_ENABLE_PROFILING})

option(GL_BUILD_DOCS "Build doxygen documentation" OFF)
set(GL_BUILD_DOCS ${GL_BUILD_DOCS})

option(GL_BUILD_DYNAMIC_LIBS "Build dynamic libraries" OFF)
set(GL_BUILD_DYNAMIC_LIBS ${GL_BUILD_DYNAMIC_LIBS})

if (GL_BUILD_DYNAMIC_LIBS)
	add_compile_definitions(GL_EXPORT)
endif()

# append into module path for our own configurations
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CMakeOptions)

set(VENDOR_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party)

find_package(Vulkan REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	find_package(X11 REQUIRED)
else()
	set(X11_LIBRARIES)
endif()


add_subdirectory(programs)
add_subdirectory(shaders)
add_subdirectory(third_party)

# Configure lua bindings
set(FFI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/engine/glitch/scripting/ffi.lua")
set(FFI_TEMPLATE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/misc/ffi_lua_resource.h.in")
set(FFI_GENERATED_PATH "${GL_OUTPUT_DIR}/include/ffi_lua_resource.gen.h")

add_custom_command(
    OUTPUT ${FFI_GENERATED_PATH}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GL_OUTPUT_DIR}/include
    COMMAND ${CMAKE_COMMAND} -DFFI_LUA_FILE:STRING="${FFI_PATH}"
                            -DINPUT_FILE:STRING="${FFI_TEMPLATE_PATH}"
                            -DOUTPUT_FILE:STRING="${FFI_GENERATED_PATH}"
                            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenFFI.cmake"
    DEPENDS ${FFI_PATH} ${FFI_TEMPLATE_PATH}
    COMMENT "Generating ${FFI_GENERATED_PATH} from ${FFI_PATH}"
)

file(GLOB_RECURSE SOURCES
	"${CMAKE_CURRENT_LIST_DIR}/engine/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/engine/*.cpp"
)

list(APPEND SOURCES ${IMGUI_FILES})
list(APPEND SOURCES ${SPIRV_REFLECT_FILES})

if (GL_BUILD_DYNAMIC_LIBS)
	add_library(glitch SHARED ${SOURCES})
else()
	add_library(glitch STATIC ${SOURCES})
endif()

target_sources(glitch PRIVATE ${FFI_GENERATED_PATH})

target_include_directories(glitch
	PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/engine
	${GLM_INCLUDES}
	PRIVATE
	${VENDOR_DIR}/include # header only libraries
	${GL_OUTPUT_DIR}/include
	${GLFW_INCLUDES}
	${SPIRV_REFLECT_INCLUDES}
	${TINY_GLTF_INCLUDES}
	${IMGUI_INCLUDES}
	${LUA_INCLUDES}
	${Vulkan_INCLUDE_DIRS}
)


target_link_libraries(glitch
	PUBLIC
	${GLFW_LIBS}
	${VK_BOOTSTRAP_LIBS}
	${LUA_LIBS}
	${Vulkan_LIBRARIES}
	${X11_LIBRARIES} # Required for GLFW on linux platform
)

target_precompile_headers(glitch PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/engine/glitch/pch.h
)

target_compile_definitions(glitch PRIVATE
	GLFW_INCLUDE_NONE
	${IMGUI_DEFINITIONS}
)

if (GL_ENABLE_PROFILING)
	target_include_directories(glitch PRIVATE ${TRACY_INCLUDES})
	target_link_libraries(glitch PRIVATE ${TRACY_LIBS})
	target_compile_definitions(glitch PRIVATE GL_ENABLE_PROFILING)
endif()

if (GL_BUILD_DOCS AND DOXYGEN_FOUND)
	find_package(Doxygen REQUIRED)

	# Configure Doxygen
	set(DOXYGEN_GENERATE_HTML YES)
	set(DOXYGEN_MARKDOWN_SUPPORT YES)
	set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_LIST_DIR}/README.md")
	set(DOXYGEN_GENERATE_TREEVIEW YES)
	set(DOXYGEN_GENERATE_DISABLE_INDEX NO)
	set(DOXYGEN_GENERATE_FULL_SIDEBAR NO)
	set(DOXYGEN_HTML_COLORSTYLE LIGHT)
	set(DOXYGEN_HTML_EXTRA_STYLESHEET
		"${CMAKE_CURRENT_LIST_DIR}/doc/doxygen/doxygen-awesome.css"
	)

	doxygen_add_docs(doxygen
		${CMAKE_CURRENT_LIST_DIR}/include
		${CMAKE_CURRENT_LIST_DIR}/README.md
	)
endif()

if (GL_BUILD_EDITOR)
	add_subdirectory(editor)
endif()

if (GL_BUILD_TESTS)
	add_subdirectory(tests)
endif()
