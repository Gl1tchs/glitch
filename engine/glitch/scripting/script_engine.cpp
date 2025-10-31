#include "glitch/scripting/script_engine.h"

#include <lua.hpp>

namespace gl {

static lua_State* s_lua = nullptr;

void ScriptEngine::init() {
	s_lua = luaL_newstate();

	luaL_openlibs(s_lua);
}

void ScriptEngine::shutdown() { lua_close(s_lua); }

Result<ScriptRef, ScriptResult> ScriptEngine::load_script_file(
		const fs::path& p_path) {
	// load the script file
	if (luaL_loadfile(s_lua, p_path.string().c_str()) != LUA_OK) {
		GL_LOG_ERROR("[LUA] Error loading script {}: {}", p_path.string(),
				lua_tostring(s_lua, -1));
		lua_pop(s_lua, 1); // pop error message

		return make_err<ScriptRef>(ScriptResult::LOAD_ERROR);
	}

	// now, execute the loaded chunk.
	if (lua_pcall(s_lua, 0, 1, 0) != LUA_OK) {
		GL_LOG_ERROR("[LUA] Error running script {}: {}", p_path.string(),
				lua_tostring(s_lua, -1));
		lua_pop(s_lua, 1); // pop error message

		return make_err<ScriptRef>(ScriptResult::EXECUTION_ERROR);
	}

	// the script should have returned a table.
	if (!lua_istable(s_lua, -1)) {
		GL_LOG_ERROR(
				"[LUA] Script {} did not return a table.", p_path.string());

		return make_err<ScriptRef>(ScriptResult::INVALID_TABLE);
	}

	// Store this table in the registry and get a reference
	return luaL_ref(s_lua, LUA_REGISTRYINDEX);
}

Result<ScriptRef, ScriptResult> ScriptEngine::load_script(
		const std::string& p_script) {
	// load and run the script
	if (luaL_dostring(s_lua, p_script.c_str()) != LUA_OK) {
		GL_LOG_ERROR("[LUA] Error running string: {}", lua_tostring(s_lua, -1));
		lua_pop(s_lua, 1); // pop error message

		return make_err<ScriptRef>(ScriptResult::LOAD_ERROR);
	}

	// The script should have returned a table.
	if (!lua_istable(s_lua, -1)) {
		GL_LOG_ERROR("[LUA] Script string did not return a table.");

		return make_err<ScriptRef>(ScriptResult::INVALID_TABLE);
	}

	// Store this table in the registry and get a reference
	return luaL_ref(s_lua, LUA_REGISTRYINDEX);
}

void ScriptEngine::get_script_by_ref(ScriptRef p_ref) {
	lua_rawgeti(s_lua, LUA_REGISTRYINDEX, p_ref);
}

void ScriptEngine::unload_script(ScriptRef p_ref) {
	if (p_ref != LUA_NOREF) {
		luaL_unref(s_lua, LUA_REGISTRYINDEX, p_ref);
	}
}

bool ScriptEngine::push_function(const char* p_func_name) {
	lua_getfield(s_lua, -1, p_func_name);
	if (!lua_isfunction(s_lua, -1)) {
		return false;
	}
	return true;
}

void ScriptEngine::push_value(int p_idx) { lua_pushvalue(s_lua, p_idx); }

void ScriptEngine::push_arg(int p_value) { lua_pushinteger(s_lua, p_value); }

void ScriptEngine::push_arg(float p_value) { lua_pushnumber(s_lua, p_value); }

void ScriptEngine::push_arg(double p_value) { lua_pushnumber(s_lua, p_value); }

void ScriptEngine::push_arg(bool p_value) { lua_pushboolean(s_lua, p_value); }

void ScriptEngine::push_arg(const char* p_value) {
	lua_pushstring(s_lua, p_value);
}

void ScriptEngine::pop_stack(int p_idx) { lua_pop(s_lua, p_idx); }

bool ScriptEngine::call_function(int p_nargs) {
	return lua_pcall(s_lua, p_nargs, 0, 0) == LUA_OK;
}

std::string ScriptEngine::get_error() {
	const std::string err = lua_tostring(s_lua, -1);
	lua_pop(s_lua, 1); // pop error
	return err;
}

} //namespace gl
