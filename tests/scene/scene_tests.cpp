#include <catch2/catch_all.hpp>

#include "core/transform.h"

#include "scene/scene.h"
#include "scene/view.h"

TEST_CASE("Scene entity creation and destruction", "[Scene]") {
	Scene scene;

	SECTION("Create new entities") {
		Entity e1 = scene.create();
		Entity e2 = scene.create();

		REQUIRE(e1 != e2); // Each entity should have a unique ID
		REQUIRE(scene.is_valid(e1));
		REQUIRE(scene.is_valid(e2));
	}

	SECTION("Destroy entities and reuse IDs") {
		Entity e1 = scene.create();
		scene.destroy(e1);

		REQUIRE_FALSE(scene.is_valid(e1)); // e1 should no longer be valid

		Entity e2 = scene.create();
		REQUIRE(scene.is_valid(e2));
		REQUIRE(get_entity_index(e2) ==
				get_entity_index(e1)); // ID of e1 should be reused
	}

	SECTION("Destroy and create multiple entities") {
		Entity e1 = scene.create();
		Entity e2 = scene.create();
		scene.destroy(e1);
		scene.destroy(e2);

		REQUIRE_FALSE(scene.is_valid(e1));
		REQUIRE_FALSE(scene.is_valid(e2));

		Entity e3 = scene.create();
		Entity e4 = scene.create();

		REQUIRE(scene.is_valid(e3));
		REQUIRE(scene.is_valid(e4));

		REQUIRE((get_entity_index(e3) == get_entity_index(e1) ||
				get_entity_index(e3) ==
						get_entity_index(
								e2))); // e3 should reuse one of the deleted IDs
		REQUIRE((get_entity_index(e4) == get_entity_index(e1) ||
				get_entity_index(e4) ==
						get_entity_index(
								e2))); // e4 should reuse the other deleted ID

		REQUIRE(get_entity_version(e3) == 1);
		REQUIRE(get_entity_version(e4) == 1);
	}

	SECTION("Check invalid entities") {
		Entity e1 = scene.create();
		REQUIRE(scene.is_valid(e1));

		Entity invalid_entity = e1 + 1000;
		REQUIRE_FALSE(scene.is_valid(invalid_entity));

		scene.destroy(e1);
		REQUIRE_FALSE(scene.is_valid(e1));
	}
}

struct TestComponent1 {
	int a;
	int b;
	int c;
};

struct TestComponent2 {
	float x;
};

TEST_CASE("Components", "[Scene]") {
	SECTION("Component ids") {
		uint32_t transform_id = get_component_id<Transform>();
		uint32_t test_component1_id = get_component_id<TestComponent1>();
		uint32_t test_component2_id = get_component_id<TestComponent2>();

		REQUIRE(transform_id != test_component1_id);
		REQUIRE(transform_id != test_component2_id);
		REQUIRE(test_component1_id != test_component2_id);
	}

	SECTION("Component assign and remove") {
		Scene scene;

		Entity e1 = scene.create();
		Entity e2 = scene.create();

		{
			TestComponent1* t1 = scene.assign<TestComponent1>(e1);
			t1->a = 6;
			t1->b = 3;
			t1->c = 9;

			REQUIRE(t1 == scene.get<TestComponent1>(e1));
		}
		{
			TestComponent2* t1 = scene.assign<TestComponent2>(e2);
			t1->x = 9.0f;

			REQUIRE(t1 == scene.get<TestComponent2>(e2));

			scene.remove<TestComponent2>(e2);

			REQUIRE_FALSE(scene.get<TestComponent2>(e2));
		}
	}
}

TEST_CASE("Scene views", "[Scene]") {
	Scene scene;

	Entity e1 = scene.create();
	Entity e2 = scene.create();
	Entity e3 = scene.create();

	scene.assign<TestComponent1>(e1);
	scene.assign<TestComponent1>(e2);
	scene.assign<TestComponent1>(e3);

	scene.assign<TestComponent2>(e1);
	scene.assign<TestComponent2>(e2);

	const auto view1 = SceneView<TestComponent1>(scene);
	{
		auto it = view1.begin();

		REQUIRE(*it == e1);

		++it;

		REQUIRE(*it == e2);

		++it;

		REQUIRE(*it == e3);

		++it;

		REQUIRE(it == view1.end());
	}

	const auto view2 = SceneView<TestComponent2>(scene);
	{
		auto it = view2.begin();

		REQUIRE(*it == e1);

		++it;

		REQUIRE(*it == e2);

		++it;

		REQUIRE(it == view2.end());
	}

	const auto view3 = SceneView(scene);
	{
		auto it = view3.begin();

		REQUIRE(*it == e1);

		++it;

		REQUIRE(*it == e2);

		++it;

		REQUIRE(*it == e3);

		++it;

		REQUIRE(it == view3.end());
	}
}
