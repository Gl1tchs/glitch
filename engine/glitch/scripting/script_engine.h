/**
 * @file script_engine.h
 */

namespace gl {

class ScriptEngine {
public:
	static void init();
	static void shutdown();

	static void compile(const std::string& p_script);
};

} //namespace gl
