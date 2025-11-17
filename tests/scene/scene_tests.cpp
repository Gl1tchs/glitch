#include <doctest/doctest.h>

#include "glitch/asset/asset_system.h"
#include "glitch/platform/os.h"
#include "glitch/scene/scene.h"

using namespace gl;

TEST_CASE("Scene entity relations") {
	Scene scene;

	Entity e1 = scene.create("E1");
	Entity e2 = scene.create("E2");

	Entity e3 = scene.create("E3", e1.get_uid());
	Entity e4 = scene.create("E4", e1.get_uid());
	Entity e5 = scene.create("E5", e2.get_uid());

	CHECK(e1.is_parent());
	CHECK(Entity::is_parent_of(e1, e3));
	CHECK(Entity::is_parent_of(e1, e4));
	CHECK(!Entity::is_parent_of(e1, e5));

	CHECK(e4.get_parent() == e1);

	CHECK(e2.get_children().front() == e5);
	CHECK(e5.get_parent() == e2);
}

TEST_CASE("Scene copy") {
	Scene scene;

	Entity e1 = scene.create("E1");
	Entity e2 = scene.create("E2");

	Entity e3 = scene.create("E3", e1.get_uid());
	Entity e4 = scene.create("E4", e1.get_uid());
	Entity e5 = scene.create("E5", e2.get_uid());

	Scene scene2;
	scene.copy_to(scene2);

	Entity e1_copy = scene2.find_by_id(e1.get_uid()).value_or(INVALID_ENTITY);
	Entity e2_copy = scene2.find_by_id(e2.get_uid()).value_or(INVALID_ENTITY);
	Entity e3_copy = scene2.find_by_id(e3.get_uid()).value_or(INVALID_ENTITY);
	Entity e4_copy = scene2.find_by_id(e4.get_uid()).value_or(INVALID_ENTITY);
	Entity e5_copy = scene2.find_by_id(e5.get_uid()).value_or(INVALID_ENTITY);

	CHECK(e1_copy != INVALID_ENTITY);
	CHECK(e2_copy != INVALID_ENTITY);
	CHECK(e3_copy != INVALID_ENTITY);
	CHECK(e4_copy != INVALID_ENTITY);
	CHECK(e5_copy != INVALID_ENTITY);

	CHECK(e1_copy.is_parent());
	CHECK(e1_copy.get_children().size() == 2);

	CHECK(e2_copy.is_parent());
	CHECK(e2_copy.get_children().size() == 1);

	CHECK(e4_copy.is_child());
	CHECK(e5_copy.is_child());
	CHECK(e5_copy.is_child());

	// CHECK(scene2.find_by_id(e2.get_uid()).get_children().front() !=
	// 		scene2.find_by_id(e5.get_uid()));
}

TEST_CASE("Scene Serialization") {
	os::setenv("GL_WORKING_DIR", fs::temp_directory_path().string().c_str());

	Scene scene;
	Scene::serialize("res://scene.glscene", scene);

	auto scene_path = AssetSystem::get_absolute_path("res://scene.glscene");
	REQUIRE(scene_path.has_value());

	CHECK(fs::exists(scene_path.get_value()));

	fs::remove(scene_path.get_value());

	CHECK(!fs::exists(scene_path.get_value()));

	os::setenv("GL_WORKING_DIR", "");
}