#include <catch2/catch_test_macros.hpp>

#include "glitch/scene/scene.h"
#include "glitch/scripting/script.h"
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

	GL_LOG_INFO("Error is expected:");
	Result<ScriptRef, ScriptResult> res = ScriptEngine::load_script(SYNTAX_ERROR);

	REQUIRE(res.is_error());
	REQUIRE(res.error() == ScriptResult::LOAD_ERROR);

	const char* TABLE_ERROR = R"(
		local Player = {}

		function Player:on_create()
			print("Player script created")
		end

		-- do not return anything
	)";

	GL_LOG_INFO("Error is expected:");
	res = ScriptEngine::load_script(TABLE_ERROR);

	REQUIRE(res.is_error());
	REQUIRE(res.error() == ScriptResult::INVALID_TABLE);

	const char* VALID_SCRIPT = R"(
		local Player = {}

		function Player:on_create()
			print("Player script created")
		end

		return Player
	)";

	res = ScriptEngine::load_script(VALID_SCRIPT);

	REQUIRE(res.is_ok());
	REQUIRE(res.value() != 0);

	REQUIRE(ScriptEngine::exec_function(res.value(), "on_create") == ScriptResult::SUCCESS);
	REQUIRE(ScriptEngine::exec_function(res.value(), "on_update") ==
			ScriptResult::FUNCTION_NOT_FOUND);

	ScriptEngine::shutdown();
}

TEST_CASE("Calling engine code") {
	ScriptEngine::init();

	std::shared_ptr<Scene> scene = std::make_shared<Scene>();

	Entity e = scene->create("Entity");

	Script* sc = e.add_component<Script>();
	sc->script_path = "tests/scripting/lua/test_basic.lua";

	scene->start();
	scene->update(0.0f);
	scene->stop();

	ScriptEngine::shutdown();
}

TEST_CASE("Script fields") {
	ScriptEngine::init();

	std::shared_ptr<Scene> scene = std::make_shared<Scene>();

	Entity e = scene->create("Entity");

	Script* sc = e.add_component<Script>();
	sc->script_path = "tests/scripting/lua/test_fields.lua";

	REQUIRE(sc->load() == ScriptResult::SUCCESS);

	ScriptMetadata metadata = ScriptEngine::get_metadata(sc->script);
	REQUIRE(metadata.fields.size() == 3);

	REQUIRE(metadata.fields.find("name") != metadata.fields.end());
	REQUIRE(metadata.fields.find("health") != metadata.fields.end());
	REQUIRE(metadata.fields.find("alive") != metadata.fields.end());

	REQUIRE(std::holds_alternative<std::string>(metadata.fields["name"]));
	REQUIRE(std::holds_alternative<double>(metadata.fields["health"]));
	REQUIRE(std::holds_alternative<bool>(metadata.fields["alive"]));

	REQUIRE("Player" == std::get<std::string>(metadata.fields["name"]));
	REQUIRE(0.0 == std::get<double>(metadata.fields["health"]));
	REQUIRE(false == std::get<bool>(metadata.fields["alive"]));

	scene->start();

	metadata = ScriptEngine::get_metadata(sc->script);

	REQUIRE("Player" == std::get<std::string>(metadata.fields["name"]));
	REQUIRE(100.0 == std::get<double>(metadata.fields["health"]));
	REQUIRE(true == std::get<bool>(metadata.fields["alive"]));

	scene->update(0.0f);

	metadata = ScriptEngine::get_metadata(sc->script);

	REQUIRE("Player1" == std::get<std::string>(metadata.fields["name"]));
	REQUIRE(80.0 == std::get<double>(metadata.fields["health"]));
	REQUIRE(true == std::get<bool>(metadata.fields["alive"]));

	// Destroys and resets the data
	scene->stop();

	metadata = ScriptEngine::get_metadata(sc->script);

	REQUIRE("Player" == std::get<std::string>(metadata.fields["name"]));
	REQUIRE(0.0 == std::get<double>(metadata.fields["health"]));
	REQUIRE(false == std::get<bool>(metadata.fields["alive"]));

	sc->unload();

	REQUIRE(!sc->is_loaded);
	REQUIRE(sc->script == 0);

	ScriptEngine::shutdown();
}
