#include "glitch/scripting/script_engine.h"

#include "glitch/core/templates/variant_helpers.h"

#include <lua.hpp>

namespace gl {

void to_json(json& j, const ScriptMetadata& metadata) {
	for (const auto& [name, field] : metadata.fields) {
		json sub_j;
		sub_j["name"] = name;
		std::visit(VariantOverloaded{
						   [&](const double& arg) { sub_j["value"] = arg; },
						   [&](const std::string& arg) { sub_j["value"] = arg; },
						   [&](const bool& arg) { sub_j["value"] = arg; },
				   },
				field);

		j["fields"].push_back(sub_j);
	}
}

void from_json(const json& j, ScriptMetadata& metadata) {
	for (const auto& sub_j : j["fields"]) {
		if (!sub_j.contains("name") || !sub_j.contains("value")) {
			GL_LOG_ERROR("[from_json] Unable to deserialize field: name or value does not exist "
						 "in json.");
			continue;
		}

		std::string name = sub_j["name"].get<std::string>();
		ScriptValueType value;
		// TODO use an enum maybe
		if (sub_j["value"].is_number()) {
			value = sub_j["value"].get<double>();
		} else if (sub_j["value"].is_string()) {
			value = sub_j["value"].get<std::string>();
		} else if (sub_j["value"].is_boolean()) {
			value = sub_j["value"].get<bool>();
		} else {
			GL_LOG_ERROR("[from_json] Unable to deserialize field '{}' for type '{}'", name,
					sub_j["value"].type_name());
			continue;
		}

		metadata.fields[name] = value;
	}
}

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

Result<ScriptRef, ScriptResult> ScriptEngine::load_script_file(const std::filesystem::path& path) {
	// check the file
	if (!std::filesystem::exists(path)) {
		GL_LOG_ERROR(
				"[LUA] [ScriptEngine::load_script_file] Script file at path '{}' does not exists.",
				path.string());
		return make_err<ScriptRef>(ScriptResult::INVALID_SCRIPT_FILE);
	}

	// load the script file
	if (luaL_loadfile(s_lua, path.string().c_str()) != LUA_OK) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::load_script_file] Error loading script {}: {}",
				path.string(), lua_tostring(s_lua, -1));
		lua_pop(s_lua, 1); // pop error message

		return make_err<ScriptRef>(ScriptResult::LOAD_ERROR);
	}

	// now, execute the loaded chunk.
	if (lua_pcall(s_lua, 0, 1, 0) != LUA_OK) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::load_script_file] Error running script {}: {}",
				path.string(), lua_tostring(s_lua, -1));
		lua_pop(s_lua, 1); // pop error message

		return make_err<ScriptRef>(ScriptResult::EXECUTION_ERROR);
	}

	// the script should have returned a table.
	if (!lua_istable(s_lua, -1)) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::load_script_file] Script at path '{}' did not return a "
					 "table.",
				path.string());

		return make_err<ScriptRef>(ScriptResult::INVALID_TABLE);
	}

	// Store this table in the registry and get a reference
	return luaL_ref(s_lua, LUA_REGISTRYINDEX);
}

Result<ScriptRef, ScriptResult> ScriptEngine::load_script(const std::string& script) {
	// load and run the script
	if (luaL_dostring(s_lua, script.c_str()) != LUA_OK) {
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

void ScriptEngine::push_script(ScriptRef ref) { lua_rawgeti(s_lua, LUA_REGISTRYINDEX, ref); }

void ScriptEngine::unload_script(ScriptRef ref) {
	if (ref != LUA_NOREF) {
		luaL_unref(s_lua, LUA_REGISTRYINDEX, ref);
	}
}

bool ScriptEngine::has_function(const char* func_name) {
	const bool res = push_function(func_name);
	pop_stack(1);
	return res;
}

bool ScriptEngine::push_function(const char* func_name) {
	lua_getfield(s_lua, -1, func_name);
	if (!lua_isfunction(s_lua, -1)) {
		return false;
	}
	return true;
}

void ScriptEngine::push_value(int idx) { lua_pushvalue(s_lua, idx); }

void ScriptEngine::push_arg(int value) { lua_pushinteger(s_lua, value); }

void ScriptEngine::push_arg(uint32_t value) { lua_pushinteger(s_lua, value); }

void ScriptEngine::push_arg(float value) { lua_pushnumber(s_lua, value); }

void ScriptEngine::push_arg(double value) { lua_pushnumber(s_lua, value); }

void ScriptEngine::push_arg(bool value) { lua_pushboolean(s_lua, value); }

void ScriptEngine::push_arg(const char* value) { lua_pushstring(s_lua, value); }

void ScriptEngine::pop_stack(int n) { lua_pop(s_lua, n); }

bool ScriptEngine::call_function(int nargs) { return lua_pcall(s_lua, nargs, 0, 0) == LUA_OK; }

ScriptMetadata ScriptEngine::get_metadata(ScriptRef ref) {
	if (ref == 0) {
		return {};
	}

	push_script(ref); // Stack: [table]

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

ScriptResult ScriptEngine::set_metadata(ScriptRef ref, const ScriptMetadata& metadata) {
	if (ref == 0) {
		return {};
	}

	push_script(ref); // Stack: [table]

	for (const auto& [name, field] : metadata.fields) {
		std::visit(VariantOverloaded{
						   [&](const double& arg) { lua_pushnumber(s_lua, arg); },
						   [&](const std::string& arg) { lua_pushstring(s_lua, arg.c_str()); },
						   [&](const bool& arg) { lua_pushboolean(s_lua, arg); },
				   },
				field);

		// Set the field: table[field_name] = value
		// This pops the value but leaves the table.
		lua_setfield(s_lua, -2, name.c_str());
	}

	pop_stack(1); // Stack: []s

	return ScriptResult::SUCCESS;
}

std::optional<double> ScriptEngine::get_number_field(ScriptRef ref, const char* field_name) {
	if (ref == 0) {
		return {};
	}

	push_script(ref);

	lua_getfield(s_lua, -1, field_name);

	if (lua_isnumber(s_lua, -1)) {
		const float value = (float)lua_tonumber(s_lua, -1);
		pop_stack(2); // Pop the value, then pop the table
		return value;
	}

	pop_stack(2); // Pop the value (nil), then pop the table

	return {};
}

std::optional<std::string> ScriptEngine::get_string_field(ScriptRef ref, const char* field_name) {
	if (ref == 0) {
		return {};
	}

	push_script(ref);

	lua_getfield(s_lua, -1, field_name);

	if (lua_isstring(s_lua, -1)) {
		std::string value = lua_tostring(s_lua, -1);
		pop_stack(2); // Pop the value, then pop the table
		return value;
	}

	pop_stack(2); // Pop the value (nil), then pop the table

	return {};
}

std::optional<bool> ScriptEngine::get_bool_field(ScriptRef ref, const char* field_name) {
	if (ref == 0) {
		return {};
	}

	push_script(ref);

	lua_getfield(s_lua, -1, field_name);

	if (lua_isboolean(s_lua, -1)) {
		const bool value = (bool)lua_toboolean(s_lua, -1);
		pop_stack(2); // Pop the value, then pop the table
		return value;
	}

	pop_stack(2); // Pop the value (nil), then pop the table

	return {};
}

bool ScriptEngine::set_field(ScriptRef ref, const char* field_name, ScriptValueType value) {
	bool result;
	std::visit(VariantOverloaded{
					   [&](double& arg) { result = ScriptEngine::set_field(ref, field_name, arg); },
					   [&](std::string& arg) {
						   result = ScriptEngine::set_field(ref, field_name, arg);
					   },
					   [&](bool& arg) { result = ScriptEngine::set_field(ref, field_name, arg); } },
			value);

	return result;
}

bool ScriptEngine::set_field(ScriptRef ref, const char* field_name, double value) {
	if (ref == 0) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::set_field] Script reference is not set.");
		return false;
	}

	push_script(ref); // Push the script table (index -1)

	lua_pushnumber(s_lua, value); // Push the new value (index -1)

	// Set the field: table[field_name] = value
	// This pops the value but leaves the table.
	lua_setfield(s_lua, -2, field_name);

	pop_stack(1); // pop the table

	return true;
}

bool ScriptEngine::set_field(ScriptRef ref, const char* field_name, const std::string& value) {
	if (ref == 0) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::set_field] Script reference is not set.");
		return false;
	}

	push_script(ref); // Push the script table (index -1)

	lua_pushstring(s_lua, value.c_str()); // Push the new value (index -1)

	// Set the field: table[field_name] = value
	// This pops the value but leaves the table.
	lua_setfield(s_lua, -2, field_name);

	pop_stack(1); // pop the table

	return true;
}

bool ScriptEngine::set_field(ScriptRef ref, const char* field_name, bool value) {
	if (ref == 0) {
		GL_LOG_ERROR("[LUA] [ScriptEngine::set_field] Script reference is not set.");
		return false;
	}

	push_script(ref); // Push the script table (index -1)

	lua_pushboolean(s_lua, value); // Push the new value (index -1)

	// Set the field: table[field_name] = value
	// This pops the value but leaves the table.
	lua_setfield(s_lua, -2, field_name);

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
