#include <lua.hpp>

#include "glitch/core/transform.h"
#include "glitch/scene/scene.h"
#include "glitch/scripting/script_system.h"

namespace gl {

extern "C" {

// Lua: Engine.Log("Hello")
void Log(const char* message) { GL_LOG_INFO("[LUA] {}", message); }

// Lua: local transform = Engine.GetTransform(entity_id)
Transform* GetTransform(uint64_t p_uid) {
	Entity entity = ScriptSystem::get_scene()->find_by_id(p_uid);
	if (entity) {
		return &entity.get_transform();
	}
	return nullptr;
}
}

static void _run_string(lua_State* L, const std::string& p_code) {
	if (luaL_dostring(L, p_code.c_str()) != LUA_OK) {
		GL_LOG_ERROR("[LUA] FFI Bind Error: {}", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

void _register_bindings(lua_State* L) {
	_run_string(L, "ffi = require('ffi')");

	_run_string(L, R"lua(
        ffi.cdef[[
            typedef struct { float x, y, z; } Vec3;
            
            typedef struct {
                Vec3 position;
                Vec3 rotation;
                Vec3 scale;
            } Transform;

            void Log(const char* message);
            Transform* GetTransform(uint32_t entity_id);
        ]]
    )lua");

	// New empty table for namespacing
	_run_string(L, "Engine = {}");

	// Bind Engine.Log
	uintptr_t log_ptr = (uintptr_t)&Log;
	_run_string(L,
			"Engine.Log = ffi.cast('void (*)(const char*)', " +
					std::to_string(log_ptr) + "ULL)");

	// Bind Engine.GetTransform
	uintptr_t get_transform_ptr = (uintptr_t)&GetTransform;
	_run_string(L,
			"Engine.GetTransform = ffi.cast('Transform* "
			"(*)(uint32_t)', " +
					std::to_string(get_transform_ptr) + "ULL)");
}

} //namespace gl