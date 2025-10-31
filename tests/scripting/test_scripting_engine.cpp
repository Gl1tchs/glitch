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

	Result<ScriptRef, ScriptResult> res =
			ScriptEngine::load_script(SYNTAX_ERROR);

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

	CHECK(ScriptEngine::exec_function(res.get_value(), "on_create") ==
			ScriptResult::SUCCESS);
	CHECK(ScriptEngine::exec_function(res.get_value(), "on_update") ==
			ScriptResult::FUNCTION_NOT_FOUND);

	ScriptEngine::shutdown();
}

TEST_CASE("Calling engine code") {
	ScriptEngine::init();

	Scene scene;

	auto e = scene.create("Entity");

	ScriptComponent& sc = e.add_component<ScriptComponent>();

	ScriptEngine::shutdown();
}
