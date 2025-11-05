#include <doctest/doctest.h>

#include "glitch/asset/asset_system.h"
#include "glitch/platform/os.h"

using namespace gl;

TEST_CASE("Test asset path conversion") {
	CHECK(os::setenv("GL_WORKING_DIR", "/home/glitch"));

	CHECK(*AssetSystem::get_absolute_path("res://script.lua") == "/home/glitch/script.lua");

	CHECK(AssetSystem::get_absolute_path("").get_error() == PathProcessError::EMPTY_PATH);
	CHECK(AssetSystem::get_absolute_path("ref://script.lua").get_error() ==
			PathProcessError::INVALID_IDENTIFIER);

	CHECK(os::setenv("GL_WORKING_DIR", ""));

	CHECK(AssetSystem::get_absolute_path("res://script.lua").get_error() ==
			PathProcessError::UNDEFINED_WORKING_DIR);
}

TEST_CASE("Asset loading / registration / freing") {
	// TODO!
}