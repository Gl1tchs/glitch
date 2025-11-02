#include <doctest.h>

#include "glitch/scene/components.h"
#include "glitch/scene/scene.h"
#include "glitch/scripting/script_engine.h"

using namespace gl;

TEST_CASE("Test script bindings") {
	ScriptEngine::init();

	Ref<Scene> scene = create_ref<Scene>();

	Entity e = scene->create("Entity");

	ScriptComponent& sc = e.add_component<ScriptComponent>();
	sc.script_path = "../tests/scripting/lua/test_bindings.lua";

	CHECK(sc.load() == ScriptResult::SUCCESS);

	scene->start();

	sc.unload();

	CHECK(!sc.is_loaded);
	CHECK(sc.script == 0);

	ScriptEngine::shutdown();
}
