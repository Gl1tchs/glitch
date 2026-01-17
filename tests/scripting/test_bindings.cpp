#include <catch2/catch_test_macros.hpp>

#include "glitch/scene/scene.h"
#include "glitch/scripting/script.h"
#include "glitch/scripting/script_engine.h"
#include "glitch/scripting/script_system.h"

using namespace gl;

TEST_CASE("Test script bindings") {
	ScriptEngine::init();

	Scene scene;

	Entity e = scene.create("Entity");

	Script* sc = e.add_component<Script>();
	sc->script_path = "tests/scripting/lua/test_bindings.lua";

	REQUIRE(sc->load() == ScriptResult::SUCCESS);

	ScriptSystem::on_runtime_start(&scene);

	REQUIRE(ScriptEngine::exec_function(sc->script, "test_bindings", e.get_uid().value) ==
			ScriptResult::SUCCESS);

	sc->unload();

	REQUIRE(!sc->is_loaded);
	REQUIRE(sc->script == 0);

	ScriptEngine::shutdown();
}
