#include <lua.hpp>

#include "glitch/core/event/input.h"
#include "glitch/core/transform.h"
#include "glitch/scene/scene.h"
#include "glitch/scripting/script_system.h"

#include "ffi_lua_resource.gen.h"

namespace gl {

std::mutex g_script_mutex;

extern "C" {

// Lua: Engine.Log("Hello")
void Log(const char* message) { GL_LOG_INFO("[LUA] {}", message); }

// Lua: local transform = Engine.GetTransform(entity_id)
Transform* GetTransform(uint64_t p_uid) {
	std::lock_guard<std::mutex> lock(g_script_mutex);

	Entity entity = ScriptSystem::get_scene()->find_by_id(p_uid);
	if (entity) {
		return &entity.get_transform();
	}
	return nullptr;
}

bool GetKeyDown(KeyCode key_code) { return Input::is_key_pressed(key_code); }

bool GetKeyUp(KeyCode key_code) { return Input::is_key_released(key_code); }

bool GetMouseDown(MouseButton mouse_code) {
	return Input::is_mouse_pressed(mouse_code);
}

bool GetMouseUp(MouseButton mouse_code) {
	return Input::is_mouse_released(mouse_code);
}
}

static void _run_string(lua_State* L, const char* p_code) {
	if (luaL_dostring(L, p_code) != LUA_OK) {
		GL_LOG_ERROR("[LUA] FFI Bind Error: {}", lua_tostring(L, -1));
		lua_pop(L, 1);
		GL_ASSERT(false); // they must work
	}
}

static void _run_string(lua_State* L, const std::string& p_code) {
	_run_string(L, p_code.c_str());
}

static void _bind_function(lua_State* L, const char* p_func_name,
		const char* p_func_def, uintptr_t p_fnptr) {
	_run_string(L,
			std::format("{} = ffi.cast('{}', {}ULL)", p_func_name, p_func_def,
					p_fnptr));
}

void _register_bindings(lua_State* L) {
	_run_string(L, lua_bindings::ffi_source);

	/* ---------------- Utility Functions ---------------- */

	_bind_function(L, "Engine.Log", "void (*)(const char*)", (uintptr_t)&Log);

	/* ---------------- Components ---------------- */

	_bind_function(L, "Engine.GetTransform", "Transform* (*)(uint32_t)",
			(uintptr_t)&GetTransform);

	/* ---------------- Input API ---------------- */

	_bind_function(
			L, "Engine.GetKeyDown", "bool (*)(int)", (uintptr_t)&GetKeyDown);
	_bind_function(L, "Engine.GetKeyUp", "bool (*)(int)", (uintptr_t)&GetKeyUp);
	_bind_function(L, "Engine.GetMouseDown", "bool (*)(int)",
			(uintptr_t)&GetMouseDown);
	_bind_function(
			L, "Engine.GetMouseUp", "bool (*)(int)", (uintptr_t)&GetMouseUp);
}

} //namespace gl