#include "glitch/scripting/script_engine.h"

#include <lua.hpp>

namespace gl {

static lua_State* s_lua = nullptr;

int _lua_print(lua_State* p_lua) {
	int n = lua_gettop(p_lua);
	for (int i = 1; i <= n; ++i) {
		if (lua_isstring(p_lua, i))
			GL_LOG_INFO("[LUA] {}", lua_tostring(p_lua, i));
		else
			GL_LOG_ERROR("[LUA] non-string arg");
	}
	return 0;
}

void ScriptEngine::init() {
	s_lua = luaL_newstate();

	luaL_openlibs(s_lua);

	lua_register(s_lua, "print", _lua_print);
}

void ScriptEngine::shutdown() { lua_close(s_lua); }

void ScriptEngine::compile(const std::string& p_script) {
	luaL_dostring(s_lua, p_script.c_str());
}

} //namespace gl
