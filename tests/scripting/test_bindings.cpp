#include <doctest.h>

#include "glitch/scene/components.h"
#include "glitch/scene/scene.h"
#include "glitch/scripting/script_engine.h"
#include "glitch/scripting/script_system.h"

using namespace gl;

TEST_CASE("Test script bindings") {
	ScriptEngine::init();

	Scene scene;

	Entity e = scene.create("Entity");

	ScriptComponent& sc = e.add_component<ScriptComponent>();
	sc.script_path = "../tests/scripting/lua/test_bindings.lua";

	CHECK(sc.load() == ScriptResult::SUCCESS);

	ScriptSystem::on_runtime_start(&scene);

	CHECK(ScriptEngine::exec_function(sc.script, "test_bindings", e.get_uid().value) ==
			ScriptResult::SUCCESS);

	sc.unload();

	CHECK(!sc.is_loaded);
	CHECK(sc.script == 0);

	ScriptEngine::shutdown();
}
