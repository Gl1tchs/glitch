/**
 * @file script_engine.h
 */

#pragma once

#include "glitch/core/core.h"

#include <unordered_map>
#include <variant>
#include <vector>

namespace gl {

typedef int ScriptRef;

enum class ScriptResult {
	SUCCESS,
	LOAD_ERROR, // script loading failed
	EXECUTION_ERROR, // problem running the actual function
	INVALID_TABLE, // the table format did not match with the engine format
	INVALID_SCRIPT_FILE, // script file could't be found
	FUNCTION_NOT_FOUND, // function could not be found
	INVALID_SCRIPT_REF,
	INVALID_FIELD_NAME,
};

typedef std::variant<double, std::string, bool> ScriptValueType;

struct ScriptMetadata {
	std::unordered_map<std::string, ScriptValueType> fields;
	std::vector<std::string> methods;
};

void to_json(json& j, const ScriptMetadata& metadata);
void from_json(const json& j, ScriptMetadata& metadata);

class ScriptEngine {
public:
	static void init();
	static void shutdown();

	/**
	 * Loads and executes the script, and stores the returned
	 * object in the Lua registry.
	 *
	 * @return ScriptRef reference key to the table in
	 * LUA_REGISTRYINDEX.
	 */
	static Result<ScriptRef, ScriptResult> load_script_file(const std::filesystem::path& path);

	/**
	 * Executes the script, and stores the returned
	 * object (should be a table) in the Lua registry.
	 * @return ScriptRef reference key to the table in LUA_REGISTRYINDEX.
	 */
	static Result<ScriptRef, ScriptResult> load_script(const std::string& script);

	/**
	 * Gets the error message and pops the error stack if a call failed
	 */
	static std::string get_error();

	template <typename... Args>
	static ScriptResult exec_function(ScriptRef table_ref, const char* func_name, Args... args) {
		if (table_ref == 0) {
			return ScriptResult::INVALID_SCRIPT_REF;
		}

		ScriptResult result = ScriptResult::SUCCESS;

		push_script(table_ref); // push the table

		if (push_function(func_name)) {
			// push the 'self' argument
			push_value(-2);

			// push args
			(push_arg(args), ...);

			// call the function (num_args = 1 (self) + sizeof...(args))
			if (!call_function(1 + sizeof...(args))) {
				GL_LOG_ERROR("[LUA] Error calling {}: {}", func_name, get_error());
				// TODO: global error handling
				result = ScriptResult::EXECUTION_ERROR;
			}
		} else {
			pop_stack(1); // pop non-function

			result = ScriptResult::FUNCTION_NOT_FOUND;
		}

		pop_stack(1); // pop the table

		return result;
	}

	/**
	 * Gets the object (table) associated with the reference.
	 * Pushes it onto the Lua stack.
	 */
	static void push_script(ScriptRef ref);

	/**
	 * Releases the script reference from the registry.
	 */
	static void unload_script(ScriptRef ref);

	static bool has_function(const char* func_name);

	static bool push_function(const char* func_name);

	static void push_value(int idx);

	static void push_arg(int value);
	static void push_arg(uint32_t value);
	static void push_arg(float value);
	static void push_arg(double value);
	static void push_arg(bool value);
	static void push_arg(const char* value);

	/**
	 * Pops `p_n` elements from the stack pointer
	 */
	static void pop_stack(int n);

	/**
	 * Call the function pushed to the stack
	 *
	 * @param nargs Number of arguments
	 */
	static bool call_function(int nargs);

	static ScriptMetadata get_metadata(ScriptRef ref);

	/**
	 * Writes metadata fields to lua
	 *
	 */
	static ScriptResult set_metadata(ScriptRef ref, const ScriptMetadata& metadata);

	static std::optional<double> get_number_field(ScriptRef ref, const char* field_name);

	static std::optional<std::string> get_string_field(ScriptRef ref, const char* field_name);

	static std::optional<bool> get_bool_field(ScriptRef ref, const char* field_name);

	static bool set_field(ScriptRef ref, const char* field_name, ScriptValueType value);

	static bool set_field(ScriptRef ref, const char* field_name, double value);
	static bool set_field(ScriptRef ref, const char* field_name, const std::string& value);
	static bool set_field(ScriptRef ref, const char* field_name, bool value);

#ifdef GL_DEBUG_BUILD
	/**
	 * @brief Dumps and logs the stack to the stdout
	 */
	static void stack_dump();
#endif
};

} //namespace gl
