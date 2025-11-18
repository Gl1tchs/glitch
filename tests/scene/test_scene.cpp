#include <doctest/doctest.h>

#include "glitch/asset/asset_system.h"
#include "glitch/platform/os.h"
#include "glitch/renderer/light_sources.h"
#include "glitch/scene/components.h"
#include "glitch/scene/scene.h"

using namespace gl;

struct MockComponent {
	AssetHandle handle;
};

struct MockAsset {
	GL_REFLECT_ASSET("MockAsset");

	int value;

	MockAsset(int v) : value(v) {}

	static std::shared_ptr<MockAsset> create(int p_value) {
		return std::make_shared<MockAsset>(p_value);
	}
};

TEST_CASE("Scene entity relations") {
	Scene scene;

	Entity e1 = scene.create("E1");
	Entity e2 = scene.create("E2");

	Entity e3 = scene.create("E3", e1);
	Entity e4 = scene.create("E4", e1);
	Entity e5 = scene.create("E5", e2);

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

	Entity e3 = scene.create("E3", e1);
	Entity e4 = scene.create("E4", e1);
	Entity e5 = scene.create("E5", e2);

	Scene scene2;
	scene.copy_to(scene2);

	Entity e1_copy = scene2.find_by_id(e1.get_uid()).value_or(INVALID_ENTITY);
	Entity e2_copy = scene2.find_by_id(e2.get_uid()).value_or(INVALID_ENTITY);
	Entity e3_copy = scene2.find_by_id(e3.get_uid()).value_or(INVALID_ENTITY);
	Entity e4_copy = scene2.find_by_id(e4.get_uid()).value_or(INVALID_ENTITY);
	Entity e5_copy = scene2.find_by_id(e5.get_uid()).value_or(INVALID_ENTITY);

	CHECK(e1_copy != INVALID_ENTITY);
	CHECK(e2_copy != INVALID_ENTITY);
	CHECK(e1_copy.get_uid() == e1.get_uid()); // UID preservation check

	CHECK(e1_copy.is_parent());
	CHECK(e1_copy.get_children().size() == 2);

	CHECK(e2_copy.is_parent());
	CHECK(e2_copy.get_children().size() == 1);

	// Verify relationship mapping in the new scene
	CHECK(e3_copy.get_parent() == e1_copy);
	CHECK(e5_copy.get_parent() == e2_copy);
}

TEST_CASE("Scene Serialization Round-Trip") {
	os::setenv("GL_WORKING_DIR", fs::temp_directory_path().string().c_str());
	const std::string scene_filename = "res://test_scene.glscene";

	std::shared_ptr<Scene> src_scene = std::make_shared<Scene>();

	AssetHandle h1 = AssetSystem::create<MockAsset>(1).value();
	AssetHandle h2 = AssetSystem::create<MockAsset>(2).value();
	AssetHandle h3 = AssetSystem::create<MockAsset>(3).value();

	Entity e1 = src_scene->create("Root Entity");
	e1.add_component<DirectionalLight>();
	e1.add_component<MockComponent>(h1);

	Entity e2 = src_scene->create("Child Light", e1);
	e2.add_component<PointLight>();
	e2.add_component<MockComponent>(h2);

	Entity e3 = src_scene->create("Child Script", e1);
	e3.add_component<CameraComponent>();
	e3.add_component<MockComponent>(h3);

	CHECK(Scene::serialize(scene_filename, src_scene));

	auto scene_path = AssetSystem::get_absolute_path(scene_filename);
	REQUIRE(scene_path.has_value());
	CHECK(fs::exists(scene_path.get_value()));

	std::shared_ptr<Scene> dst_scene = std::make_shared<Scene>();
	bool load_success = Scene::deserialize(scene_filename, dst_scene);

	REQUIRE(load_success);

	// Verify Data Integrity
	SUBCASE("Verify Entity Existence and IDs") {
		CHECK(dst_scene->exists(e1.get_uid()));
		CHECK(dst_scene->exists(e2.get_uid()));
		CHECK(dst_scene->exists(e3.get_uid()));

		// Should be 3 entities
		// (Assuming view() or a count method exists, otherwise verify by lookups)
		// CHECK(dst_scene->entity_count() == 3);
	}

	SUBCASE("Verify Hierarchy") {
		Entity e1_loaded = dst_scene->find_by_id(e1.get_uid()).value();
		Entity e2_loaded = dst_scene->find_by_id(e2.get_uid()).value();
		Entity e3_loaded = dst_scene->find_by_id(e3.get_uid()).value();

		CHECK(e1_loaded.is_parent());
		CHECK(e2_loaded.get_parent() == e1_loaded);
		CHECK(e3_loaded.get_parent() == e1_loaded);

		// Ensure E2/E3 are strictly siblings
		CHECK(e2_loaded.get_parent() == e3_loaded.get_parent());
	}

	SUBCASE("Verify Components") {
		Entity e2_loaded = dst_scene->find_by_id(e2.get_uid()).value();
		Entity e3_loaded = dst_scene->find_by_id(e3.get_uid()).value();

		// Check simple component existence
		CHECK(e2_loaded.has_component<PointLight>());
		CHECK(!e2_loaded.has_component<DirectionalLight>()); // Should not have parent's comp
		CHECK(e3_loaded.has_component<CameraComponent>());
	}

	SUBCASE("Verify Asset References") {
		Entity e1_loaded = dst_scene->find_by_id(e1.get_uid()).value();
		if (e1_loaded.has_component<MockComponent>()) {
			auto* mc = e1_loaded.get_component<MockComponent>();
			// Verify the handle is valid or IDs match
			CHECK(mc->handle.is_valid());
		}
	}

	if (fs::exists(scene_path.get_value())) {
		fs::remove(scene_path.get_value());
	}

	os::setenv("GL_WORKING_DIR", "");
}