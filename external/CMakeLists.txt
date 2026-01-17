include(FetchContent)

# ------------------- Tracy profiler -------------------
if (GL_ENABLE_PROFILING)
	FetchContent_Declare(
		tracy
		GIT_REPOSITORY https://github.com/wolfpld/tracy.git
		GIT_TAG v0.12.2 # TracyClient and tracy instance need to have the same version
	)

	FetchContent_MakeAvailable(tracy)

	set(TRACY_INCLUDES ${tracy_SOURCE_DIR}/public CACHE INTERNAL "")
	set(TRACY_LIBS TracyClient CACHE INTERNAL "")
endif()

# ------------------- GLFW -------------------
FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 3.4
)

set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

set(GLFW_INCLUDES ${glfw_SOURCE_DIR}/include CACHE INTERNAL "")
set(GLFW_LIBS glfw CACHE INTERNAL "")

# ------------------- glm -------------------
FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 1.0.2
)

FetchContent_MakeAvailable(glm)

set(GLM_INCLUDES ${glm_SOURCE_DIR} CACHE INTERNAL "")

# ------------------- ImGui -------------------
FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG docking
)

FetchContent_MakeAvailable(imgui)

set(IMGUI_INCLUDES ${imgui_SOURCE_DIR} CACHE INTERNAL "")
set(IMGUI_FILES
	"${imgui_SOURCE_DIR}/imgui.h"
	"${imgui_SOURCE_DIR}/imgui_internal.h"
	"${imgui_SOURCE_DIR}/imgui.cpp"
	"${imgui_SOURCE_DIR}/imgui_demo.cpp"
	"${imgui_SOURCE_DIR}/imgui_draw.cpp"
	"${imgui_SOURCE_DIR}/imgui_widgets.cpp"
	"${imgui_SOURCE_DIR}/imgui_tables.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
	"${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
	"${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
	CACHE INTERNAL ""
)

# ------------------- TinyGLTF -------------------
FetchContent_Declare(
	tinygltf
	GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
	GIT_TAG release
)

FetchContent_MakeAvailable(tinygltf)

set(TINY_GLTF_INCLUDES ${tinygltf_SOURCE_DIR} CACHE INTERNAL "")

# ------------------- tinyfiledialogs -------------------
add_subdirectory(tinyfiledialogs)

set(TINYFD_LIBS tinyfiledialogs CACHE INTERNAL "")

# ------------------- lua -------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(LUAJIT_PATH ${VENDOR_DIR}/luajit/build/linux-x86_64)
	
	find_library(LUAJIT_LIBRARY luajit
		PATHS ${LUAJIT_PATH}
		PATH_SUFFIXES lib
	)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	message(FATAL_ERROR "Darwin systems are not supported!")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(LUAJIT_PATH ${VENDOR_DIR}/luajit/build/win32)

	find_library(LUAJIT_LIBRARY lua51.lib
		PATHS ${LUAJIT_PATH}
		PATH_SUFFIXES lib
	)
endif()

find_path(LUAJIT_INCLUDE_DIR luajit.h PATHS ${LUAJIT_PATH})

if(NOT LUAJIT_INCLUDE_DIR OR NOT LUAJIT_LIBRARY)
    message(FATAL_ERROR "Failed to find LuaJIT headers or library at ${LUAJIT_PATH}")
endif()

message(STATUS "Found LuaJIT headers: ${LUAJIT_INCLUDE_DIR}")
message(STATUS "Found LuaJIT library: ${LUAJIT_LIBRARY}")

set(LUA_INCLUDES ${LUAJIT_INCLUDE_DIR} ${VENDOR_DIR}/luajit/src ${luajit_SOURCE_DIR} CACHE INTERNAL "")
set(LUA_LIBS ${LUAJIT_LIBRARY} CACHE INTERNAL "")
