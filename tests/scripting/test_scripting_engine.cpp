#include <doctest/doctest.h>

#include "glitch/scene/components.h"
#include "glitch/scene/scene.h"
#include "glitch/scripting/script_engine.h"

using namespace gl;

TEST_CASE("Script loading") {
	ScriptEngine::init();

	const char* SYNTAX_ERROR = R"(
		local Player = {}

		function Player:on_create( -- no parantheses
			print("Player script created")
		end

		return Player
	)";

	Result<ScriptRef, ScriptResult> res = ScriptEngine::load_script(SYNTAX_ERROR);

	CHECK(res.has_error());
	CHECK(res.get_error() == ScriptResult::LOAD_ERROR);

	const char* TABLE_ERROR = R"(
		local Player = {}

		function Player:on_create()
			print("Player script created")
		end

		-- do not return anything
	)";

	res = ScriptEngine::load_script(TABLE_ERROR);

	CHECK(res.has_error());
	CHECK(res.get_error() == ScriptResult::INVALID_TABLE);

	const char* VALID_SCRIPT = R"(
		local Player = {}

		function Player:on_create()
			print("Player script created")
		end

		return Player
	)";

	res = ScriptEngine::load_script(VALID_SCRIPT);

	CHECK(res.has_value());
	CHECK(res.get_value() != 0);

	CHECK(ScriptEngine::exec_function(res.get_value(), "on_create") == ScriptResult::SUCCESS);
	CHECK(ScriptEngine::exec_function(res.get_value(), "on_update") ==
			ScriptResult::FUNCTION_NOT_FOUND);

	ScriptEngine::shutdown();
}

TEST_CASE("Calling engine code") {
	ScriptEngine::init();

	Ref<Scene> scene = create_ref<Scene>();

	Entity e = scene->create("Entity");

	ScriptComponent& sc = e.add_component<ScriptComponent>();
	sc.script_path = "../tests/scripting/lua/test_basic.lua";

	scene->start();
	scene->update(0.0f);
	scene->stop();

	ScriptEngine::shutdown();
}

TEST_CASE("Script fields") {
	ScriptEngine::init();

	Ref<Scene> scene = create_ref<Scene>();

	Entity e = scene->create("Entity");

	ScriptComponent& sc = e.add_component<ScriptComponent>();
	sc.script_path = "../tests/scripting/lua/test_fields.lua";

	CHECK(sc.load() == ScriptResult::SUCCESS);

	ScriptMetadata metadata = ScriptEngine::get_metadata(sc.script);
	CHECK(metadata.fields.size() == 3);

	CHECK(metadata.fields.find("name") != metadata.fields.end());
	CHECK(metadata.fields.find("health") != metadata.fields.end());
	CHECK(metadata.fields.find("alive") != metadata.fields.end());

	CHECK(std::holds_alternative<std::string>(metadata.fields["name"]));
	CHECK(std::holds_alternative<double>(metadata.fields["health"]));
	CHECK(std::holds_alternative<bool>(metadata.fields["alive"]));

	CHECK("Player" == std::get<std::string>(metadata.fields["name"]));
	CHECK(0.0 == std::get<double>(metadata.fields["health"]));
	CHECK(false == std::get<bool>(metadata.fields["alive"]));

	scene->start();

	metadata = ScriptEngine::get_metadata(sc.script);

	CHECK("Player" == std::get<std::string>(metadata.fields["name"]));
	CHECK(100.0 == std::get<double>(metadata.fields["health"]));
	CHECK(true == std::get<bool>(metadata.fields["alive"]));

	scene->update(0.0f);

	metadata = ScriptEngine::get_metadata(sc.script);

	CHECK("Player1" == std::get<std::string>(metadata.fields["name"]));
	CHECK(80.0 == std::get<double>(metadata.fields["health"]));
	CHECK(true == std::get<bool>(metadata.fields["alive"]));

	// Destroys and resets the data
	scene->stop();

	metadata = ScriptEngine::get_metadata(sc.script);

	CHECK("Player" == std::get<std::string>(metadata.fields["name"]));
	CHECK(0.0 == std::get<double>(metadata.fields["health"]));
	CHECK(false == std::get<bool>(metadata.fields["alive"]));

	sc.unload();

	CHECK(!sc.is_loaded);
	CHECK(sc.script == 0);

	ScriptEngine::shutdown();
}
