#include <doctest/doctest.h>

#include "glitch/scripting/script_engine.h"

using namespace gl;

TEST_CASE("Script compilation") {
	ScriptEngine::init();

	ScriptEngine::compile("print(\"Hello World\\n\")");

	ScriptEngine::shutdown();
}

TEST_CASE("Calling engine code") {}
