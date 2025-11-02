#include "glitch/core/event/input.h"
#include "glitch/core/transform.h"
#include "glitch/scene/scene.h"
#include "glitch/scripting/script_system.h"

#include <lua.hpp>

#include "ffi_lua_resource.gen.h"

namespace gl {

std::mutex g_script_mutex;

typedef uint32_t EntityUID;

extern "C" {

// --- Global functions ---

void Log(const char* message);

EntityUID FindEntityById(EntityUID p_id);
EntityUID FindEntityByName(const char* p_name);

// --- Input ---

int GetKeyDown(int key_code);
int GetKeyUp(int key_code);
int GetMouseDown(int mouse_code);
int GetMouseUp(int mouse_code);

// --- Entity Methods  ---

Transform* GetTransform(EntityUID self);

// --- Transform Methods  ---

void Transform_SetPosition(Transform* self, glm::vec3 new_pos);
glm::vec3 Transform_GetPosition(Transform* self);

} // extern "C"

static void _run_string(lua_State* L, const char* p_code) {
	if (luaL_dostring(L, p_code) != LUA_OK) {
		GL_LOG_ERROR("[LUA] FFI Bind Error: {}", lua_tostring(L, -1));
		lua_pop(L, 1);
		GL_ASSERT(false); // FFI scripts must compile
	}
}

static void _run_string(lua_State* L, const std::string& p_code) {
	_run_string(L, p_code.c_str());
}

static void _bind_function(lua_State* L, const char* p_func_path,
		const char* p_func_def, uintptr_t p_fnptr) {
	_run_string(L,
			std::format("{} = ffi.cast('{}', {}ULL)", p_func_path, p_func_def,
					p_fnptr));
}

void _register_bindings(lua_State* L) {
	// Load and run the FFI script
	_run_string(L, lua_bindings::ffi_source);

	/* ---------------- Utility Functions ---------------- */
	_bind_function(L, "Engine.C_Functions.Log", "void (*)(const char*)",
			(uintptr_t)&Log);

	/* ---------------- Global Engine Functions ---------------- */
	_bind_function(L, "Engine.C_Functions.FindEntityById",
			"uint32_t (*)(uint32_t)", (uintptr_t)&FindEntityById);
	_bind_function(L, "Engine.C_Functions.FindEntityByName",
			"uint32_t (*)(const char*)", (uintptr_t)&FindEntityByName);

	_bind_function(L, "Engine.C_Functions.GetTransform",
			"Transform* (*)(uint32_t)", (uintptr_t)&GetTransform);

	/* ---------------- Input API ---------------- */
	_bind_function(L, "Engine.C_Functions.GetKeyDown", "int (*)(int)",
			(uintptr_t)&GetKeyDown);
	_bind_function(L, "Engine.C_Functions.GetKeyUp", "int (*)(int)",
			(uintptr_t)&GetKeyUp);
	_bind_function(L, "Engine.C_Functions.GetMouseDown", "int (*)(int)",
			(uintptr_t)&GetMouseDown);
	_bind_function(L, "Engine.C_Functions.GetMouseUp", "int (*)(int)",
			(uintptr_t)&GetMouseUp);

	/* ---------------- Transform Methods ---------------- */
	_bind_function(L, "Engine.C_Functions.Transform_SetPosition",
			"void (*)(Transform*, Vec3)", (uintptr_t)&Transform_SetPosition);
	_bind_function(L, "Engine.C_Functions.Transform_GetPosition",
			"Vec3 (*)(Transform*)", (uintptr_t)&Transform_GetPosition);
}

extern "C" {

void Log(const char* message) { GL_LOG_INFO("[LUA] {}", message); }

EntityUID FindEntityById(EntityUID p_id) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return 0;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(p_id);
	return entity ? p_id : 0;
}

EntityUID FindEntityByName(const char* p_name) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return 0;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_name(p_name);
	return entity ? (*entity).get_uid().value : 0;
}

int GetKeyDown(int key_code) {
	return Input::is_key_pressed((KeyCode)key_code);
}

int GetKeyUp(int key_code) { return Input::is_key_released((KeyCode)key_code); }

int GetMouseDown(int mouse_code) {
	return Input::is_mouse_pressed((MouseButton)mouse_code);
}

int GetMouseUp(int mouse_code) {
	return Input::is_mouse_released((MouseButton)mouse_code);
}

Transform* GetTransform(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running() || self == 0) {
		return nullptr;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (entity) {
		return &(*entity).get_transform();
	}
	return nullptr;
}

void Transform_SetPosition(Transform* self, glm::vec3 new_pos) {
	if (!self) {
		return;
	}

	std::lock_guard<std::mutex> lock(g_script_mutex);
	self->local_position = new_pos;
}

glm::vec3 Transform_GetPosition(Transform* self) {
	if (!self) {
		return { 0, 0, 0 };
	}

	std::lock_guard<std::mutex> lock(g_script_mutex);
	return self->local_position;
}

} // extern "C"
} // namespace gl
