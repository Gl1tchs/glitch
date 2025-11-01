#include <doctest/doctest.h>

#include "glitch/core/transform.h"
#include "glitch/scene/component_lookup.h"
#include "glitch/scene/registry.h"

using namespace gl;

struct TestComponent1 {
	int a;
	int b;
	int c;

	friend bool operator==(
			const TestComponent1& lhs, const TestComponent1& rhs) {
		return std::tie(lhs.a, lhs.b, lhs.c) == std::tie(rhs.a, rhs.b, rhs.c);
	}
};

struct TestComponent2 {
	float x;
};

TEST_CASE("Registry entity creation and destruction") {
	Registry scene;

	SUBCASE("Create new entities") {
		EntityId e1 = scene.spawn();
		EntityId e2 = scene.spawn();

		CHECK(e1 != e2); // Each entity should have a unique ID
		CHECK(scene.is_valid(e1));
		CHECK(scene.is_valid(e2));
	}

	SUBCASE("Destroy entities and reuse IDs") {
		EntityId e1 = scene.spawn();
		scene.despawn(e1);

		CHECK_FALSE(scene.is_valid(e1)); // e1 should no longer be valid

		EntityId e2 = scene.spawn();
		CHECK(scene.is_valid(e2));
		CHECK(get_entity_index(e2) ==
				get_entity_index(e1)); // ID of e1 should be reused
	}

	SUBCASE("Destroy and spawn multiple entities") {
		EntityId e1 = scene.spawn();
		EntityId e2 = scene.spawn();
		scene.despawn(e1);
		scene.despawn(e2);

		CHECK_FALSE(scene.is_valid(e1));
		CHECK_FALSE(scene.is_valid(e2));

		EntityId e3 = scene.spawn();
		EntityId e4 = scene.spawn();

		CHECK(scene.is_valid(e3));
		CHECK(scene.is_valid(e4));

		CHECK((get_entity_index(e3) == get_entity_index(e1) ||
				get_entity_index(e3) ==
						get_entity_index(
								e2))); // e3 should reuse one of the deleted IDs
		CHECK((get_entity_index(e4) == get_entity_index(e1) ||
				get_entity_index(e4) ==
						get_entity_index(
								e2))); // e4 should reuse the other deleted ID

		CHECK(get_entity_version(e3) == 1);
		CHECK(get_entity_version(e4) == 1);
	}

	SUBCASE("Check invalid entities") {
		EntityId e1 = scene.spawn();
		CHECK(scene.is_valid(e1));

		EntityId invalid_entity = e1 + 1000;
		CHECK_FALSE(scene.is_valid(invalid_entity));

		scene.despawn(e1);
		CHECK_FALSE(scene.is_valid(e1));
	}
}

TEST_CASE("Registry Copy") {
	Registry scene1;

	EntityId e1 = scene1.spawn();

	TestComponent1* t1 = scene1.assign<TestComponent1>(e1);
	t1->a = 1;
	t1->b = 2;
	t1->c = 3;

	scene1.assign<TestComponent2>(e1);

	EntityId e2 = scene1.spawn();
	scene1.assign<TestComponent1>(e2);

	Registry scene2;
	scene1.copy_to(scene2);

	// Entities and their components has to be copied as they are

	CHECK(scene2.has<TestComponent1>(e1));
	CHECK(scene2.has<TestComponent2>(e1));

	TestComponent1* t1_copy = scene2.get<TestComponent1>(e1);
	CHECK(t1_copy != t1); // they are not pointing to the same memory
	CHECK(*t1_copy == *t1); // but inside are the same

	CHECK(scene2.has<TestComponent1>(e2));
}

TEST_CASE("Components") {
	SUBCASE("Component ids") {
		uint32_t transform_id = get_component_id<Transform>();
		uint32_t test_component1_id = get_component_id<TestComponent1>();
		uint32_t test_component2_id = get_component_id<TestComponent2>();

		CHECK(transform_id != test_component1_id);
		CHECK(transform_id != test_component2_id);
		CHECK(test_component1_id != test_component2_id);
	}

	SUBCASE("Component assign and remove") {
		Registry scene;

		EntityId e1 = scene.spawn();
		EntityId e2 = scene.spawn();

		{
			TestComponent1* t1 =
					scene.assign<TestComponent1>(e1, 6.0f, 3.0f, 9.0f);

			CHECK(t1 == scene.get<TestComponent1>(e1));

			CHECK(t1->a == 6.0f);
			CHECK(t1->b == 3.0f);
			CHECK(t1->c == 9.0f);
		}
		{
			TestComponent2* t1 = scene.assign<TestComponent2>(e2, 9.0f);

			CHECK(t1 == scene.get<TestComponent2>(e2));
			CHECK(t1->x == 9.0f);

			scene.remove<TestComponent2>(e2);

			CHECK_FALSE(scene.get<TestComponent2>(e2));
		}
	}
}

TEST_CASE("Registry views") {
	Registry scene;

	EntityId e1 = scene.spawn();
	EntityId e2 = scene.spawn();
	EntityId e3 = scene.spawn();

	scene.assign<TestComponent1, TestComponent2>(e1);
	scene.assign<TestComponent1, TestComponent2>(e2);
	scene.assign<TestComponent1>(e3);

	const auto view1 = scene.view<TestComponent1>();
	{
		auto it = view1.begin();

		CHECK(*it == e1);

		++it;

		CHECK(*it == e2);

		++it;

		CHECK(*it == e3);

		++it;

		CHECK(it == view1.end());
	}

	const auto view2 = scene.view<TestComponent2>();
	{
		auto it = view2.begin();

		CHECK(*it == e1);

		++it;

		CHECK(*it == e2);

		++it;

		CHECK(it == view2.end());
	}

	const auto view3 = scene.view();
	{
		auto it = view3.begin();

		CHECK(*it == e1);

		++it;

		CHECK(*it == e2);

		++it;

		CHECK(*it == e3);

		++it;

		CHECK(it == view3.end());
	}
}