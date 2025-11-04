#include "glitch/scripting/script_engine.h"

#include <lua.hpp>

namespace gl {

template <class... Ts> struct overloaded : Ts... {
	using Ts::operator()...;
};

static lua_State* s_lua = nullptr;

/**
 * Register engine functionality into lua
 *
 */
extern void register_ffi_bindings(lua_State* L);

void ScriptEngine::init() {
	s_lua = luaL_newstate();

	luaL_openlibs(s_lua);

	// Expose engine functions
	register_ffi_bindings(s_lua);
}

void ScriptEngine::shutdown() { lua_close(s_lua); }

Result<ScriptRef, ScriptResult> ScriptEngine::load_script_file(const fs::path& p_path) {
	// check the file
	if (!fs::exists(p_path)) {
		GL_LOG_ERROR(
				"[LUA] [ScriptEngine::load_script_file] Script file at path '{}' does not exists.",
				p_path.string());
		return make_err<ScriptRef>(ScriptResult::INVALID_SCRIPT_FILE);
	}

	// load the script file
	if (luaL_loadfile(s_lua, p_path.string().c_str()) != LUA_OK) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::load_script_file] Error loading script {}: {}",
				p_path.string(), lua_tostring(s_lua, -1));
		lua_pop(s_lua, 1); // pop error message

		return make_err<ScriptRef>(ScriptResult::LOAD_ERROR);
	}

	// now, execute the loaded chunk.
	if (lua_pcall(s_lua, 0, 1, 0) != LUA_OK) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::load_script_file] Error running script {}: {}",
				p_path.string(), lua_tostring(s_lua, -1));
		lua_pop(s_lua, 1); // pop error message

		return make_err<ScriptRef>(ScriptResult::EXECUTION_ERROR);
	}

	// the script should have returned a table.
	if (!lua_istable(s_lua, -1)) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::load_script_file] Script at path '{}' did not return a "
					 "table.",
				p_path.string());

		return make_err<ScriptRef>(ScriptResult::INVALID_TABLE);
	}

	// Store this table in the registry and get a reference
	return luaL_ref(s_lua, LUA_REGISTRYINDEX);
}

Result<ScriptRef, ScriptResult> ScriptEngine::load_script(const std::string& p_script) {
	// load and run the script
	if (luaL_dostring(s_lua, p_script.c_str()) != LUA_OK) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::load_script] Error running string: {}",
				lua_tostring(s_lua, -1));
		lua_pop(s_lua, 1); // pop error message

		return make_err<ScriptRef>(ScriptResult::LOAD_ERROR);
	}

	// The script should have returned a table.
	if (!lua_istable(s_lua, -1)) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::load_script] Script did not return a table.");

		return make_err<ScriptRef>(ScriptResult::INVALID_TABLE);
	}

	// Store this table in the registry and get a reference
	return luaL_ref(s_lua, LUA_REGISTRYINDEX);
}

void ScriptEngine::push_script(ScriptRef p_ref) { lua_rawgeti(s_lua, LUA_REGISTRYINDEX, p_ref); }

void ScriptEngine::unload_script(ScriptRef p_ref) {
	if (p_ref != LUA_NOREF) {
		luaL_unref(s_lua, LUA_REGISTRYINDEX, p_ref);
	}
}

bool ScriptEngine::has_function(const char* p_func_name) {
	const bool res = push_function(p_func_name);
	pop_stack(1);
	return res;
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

void ScriptEngine::push_arg(uint32_t p_value) { lua_pushinteger(s_lua, p_value); }

void ScriptEngine::push_arg(float p_value) { lua_pushnumber(s_lua, p_value); }

void ScriptEngine::push_arg(double p_value) { lua_pushnumber(s_lua, p_value); }

void ScriptEngine::push_arg(bool p_value) { lua_pushboolean(s_lua, p_value); }

void ScriptEngine::push_arg(const char* p_value) { lua_pushstring(s_lua, p_value); }

void ScriptEngine::pop_stack(int p_n) { lua_pop(s_lua, p_n); }

bool ScriptEngine::call_function(int p_nargs) { return lua_pcall(s_lua, p_nargs, 0, 0) == LUA_OK; }

ScriptMetadata ScriptEngine::get_metadata(ScriptRef p_ref) {
	if (p_ref == 0) {
		return {};
	}

	push_script(p_ref); // Stack: [table]

	if (!lua_istable(s_lua, -1)) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::get_metadata] Reference is not a table.");
		pop_stack(1);
		return {};
	}

	// Push nil onto the stack. lua_next expects a key below the table.
	lua_pushnil(s_lua); // Stack: [table, nil]

	ScriptMetadata metadata = {};

	while (lua_next(s_lua, -2) != 0) { // -2 is the table index
		// Stack: [table, key, value]

		// Key is at index -2)
		std::string key_name = "";
		int key_type = lua_type(s_lua, -2);

		if (key_type == LUA_TSTRING) {
			key_name = lua_tostring(s_lua, -2);
		} else {
			// Handle non-string keys
			// TODO?
			key_name = lua_typename(s_lua, key_type);
		}

		// Value is at index -1
		const int value_type = lua_type(s_lua, -1);
		switch (value_type) {
			case LUA_TNUMBER:
				metadata.fields[key_name] = (double)lua_tonumber(s_lua, -1);
				break;
			case LUA_TSTRING:
				metadata.fields[key_name] = std::string(lua_tostring(s_lua, -1));
				break;
			case LUA_TBOOLEAN:
				metadata.fields[key_name] = (bool)lua_toboolean(s_lua, -1);
				break;
			case LUA_TFUNCTION:
				metadata.methods.push_back(key_name);
				break;
			default:
				GL_LOG_ERROR("[LUA] [ScriptEngine::get_metadata] Unsupported key of type: {}",
						lua_typename(s_lua, value_type));
				break;
		}

		// Pop the value, but leave the key for the next iteration.
		// The key must be at the top of the stack for the next lua_next call.
		pop_stack(1); // Stack: [table, key]
	}

	pop_stack(1); // Stack: []

	return metadata;
}

std::optional<double> ScriptEngine::get_number_field(ScriptRef p_ref, const char* p_field_name) {
	if (p_ref == 0) {
		return {};
	}

	push_script(p_ref);

	lua_getfield(s_lua, -1, p_field_name);

	if (lua_isnumber(s_lua, -1)) {
		const float value = (float)lua_tonumber(s_lua, -1);
		pop_stack(2); // Pop the value, then pop the table
		return value;
	}

	pop_stack(2); // Pop the value (nil), then pop the table

	return {};
}

std::optional<std::string> ScriptEngine::get_string_field(
		ScriptRef p_ref, const char* p_field_name) {
	if (p_ref == 0) {
		return {};
	}

	push_script(p_ref);

	lua_getfield(s_lua, -1, p_field_name);

	if (lua_isstring(s_lua, -1)) {
		std::string value = lua_tostring(s_lua, -1);
		pop_stack(2); // Pop the value, then pop the table
		return value;
	}

	pop_stack(2); // Pop the value (nil), then pop the table

	return {};
}

std::optional<bool> ScriptEngine::get_bool_field(ScriptRef p_ref, const char* p_field_name) {
	if (p_ref == 0) {
		return {};
	}

	push_script(p_ref);

	lua_getfield(s_lua, -1, p_field_name);

	if (lua_isboolean(s_lua, -1)) {
		const bool value = (bool)lua_toboolean(s_lua, -1);
		pop_stack(2); // Pop the value, then pop the table
		return value;
	}

	pop_stack(2); // Pop the value (nil), then pop the table

	return {};
}

bool ScriptEngine::set_field(ScriptRef p_ref, const char* p_field_name, ScriptValueType p_value) {
	bool result;
	std::visit(overloaded{ [&](double& arg) {
							  result = ScriptEngine::set_field(p_ref, p_field_name, arg);
						  },
					   [&](std::string& arg) {
						   result = ScriptEngine::set_field(p_ref, p_field_name, arg);
					   },
					   [&](bool& arg) {
						   result = ScriptEngine::set_field(p_ref, p_field_name, arg);
					   } },
			p_value);

	return result;
}

bool ScriptEngine::set_field(ScriptRef p_ref, const char* p_field_name, double p_value) {
	if (p_ref == 0) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::set_field] Script reference is not set.");
		return false;
	}

	push_script(p_ref); // Push the script table (index -1)

	lua_pushnumber(s_lua, p_value); // Push the new value (index -1)

	// Set the field: table[field_name] = value
	// This pops the value but leaves the table.
	lua_setfield(s_lua, -2, p_field_name);

	pop_stack(1); // pop the table

	return true;
}

bool ScriptEngine::set_field(
		ScriptRef p_ref, const char* p_field_name, const std::string& p_value) {
	if (p_ref == 0) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::set_field] Script reference is not set.");
		return false;
	}

	push_script(p_ref); // Push the script table (index -1)

	lua_pushstring(s_lua, p_value.c_str()); // Push the new value (index -1)

	// Set the field: table[field_name] = value
	// This pops the value but leaves the table.
	lua_setfield(s_lua, -2, p_field_name);

	pop_stack(1); // pop the table

	return true;
}

bool ScriptEngine::set_field(ScriptRef p_ref, const char* p_field_name, bool p_value) {
	if (p_ref == 0) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::set_field] Script reference is not set.");
		return false;
	}

	push_script(p_ref); // Push the script table (index -1)

	lua_pushboolean(s_lua, p_value); // Push the new value (index -1)

	// Set the field: table[field_name] = value
	// This pops the value but leaves the table.
	lua_setfield(s_lua, -2, p_field_name);

	pop_stack(1); // pop the table

	return true;
}

std::string ScriptEngine::get_error() {
	const std::string err = lua_tostring(s_lua, -1);
	lua_pop(s_lua, 1); // pop error
	return err;
}

#ifdef GL_DEBUG_BUILD
void ScriptEngine::stack_dump() {
	const int top = lua_gettop(s_lua);
	for (int i = 1; i <= top; i++) {
		// repeat for each level
		int t = lua_type(s_lua, i);
		switch (t) {
			case LUA_TSTRING:
				printf("`%s'", lua_tostring(s_lua, i));
				break;
			case LUA_TBOOLEAN:
				printf(lua_toboolean(s_lua, i) ? "true" : "false");
				break;
			case LUA_TNUMBER:
				printf("%g", lua_tonumber(s_lua, i));
				break;
			default:
				printf("%s", lua_typename(s_lua, t));
				break;
		}
		printf("  "); // put a separator
	}
	printf("\n"); // end the listing
}
#endif

} //namespace gl
