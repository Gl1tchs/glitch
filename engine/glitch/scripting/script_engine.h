/**
 * @file script_engine.h
 */

#pragma once

namespace gl {

typedef int ScriptRef;

enum class ScriptResult {
	SUCCESS,
	LOAD_ERROR, // script loading failed
	EXECUTION_ERROR, // problem running the actual function
	INVALID_TABLE, // the table format did not match with the engine format
	INVALID_SCRIPT_FILE, // script file could't be found
	FUNCTION_NOT_FOUND, // function could not be found
};

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
	static Result<ScriptRef, ScriptResult> load_script_file(
			const fs::path& p_path);

	/**
	 * Executes the script, and stores the returned
	 * object (should be a table) in the Lua registry.
	 * @return ScriptRef reference key to the table in LUA_REGISTRYINDEX.
	 */
	static Result<ScriptRef, ScriptResult> load_script(
			const std::string& p_script);

	/**
	 * Gets the error message and pops the error stack if a call failed
	 */
	static std::string get_error();

	template <typename... Args>
	static ScriptResult exec_function(
			ScriptRef p_table_ref, const char* p_func_name, Args... p_args) {
		ScriptResult result = ScriptResult::SUCCESS;

		get_script_by_ref(p_table_ref); // push the table

		if (push_function(p_func_name)) {
			// push the 'self' argument
			push_value(-2);

			// push args
			(push_arg(p_args), ...);

			// call the function (num_args = 1 (self) + sizeof...(args))
			if (!call_function(1 + sizeof...(p_args))) {
				GL_LOG_ERROR(
						"[LUA] Error calling {}: {}", p_func_name, get_error());
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
	static void get_script_by_ref(ScriptRef p_ref);

	/**
	 * Releases the script reference from the registry.
	 */
	static void unload_script(ScriptRef p_ref);

	static bool push_function(const char* p_func_name);

	static void push_value(int p_idx);

	static void push_arg(int p_value);
	static void push_arg(float p_value);
	static void push_arg(double p_value);
	static void push_arg(bool p_value);
	static void push_arg(const char* p_value);

	/**
	 * Pops the stack pointer
	 */
	static void pop_stack(int p_idx);

	/**
	 * Call the function pushed to the stack
	 *
	 * @param p_nargs Number of arguments
	 */
	static bool call_function(int p_nargs);
};

} //namespace gl
