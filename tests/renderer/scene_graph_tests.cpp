#include <catch2/catch_all.hpp>

#include "renderer/scene_graph.h"

TEST_CASE("SceneGraph Basic Operations", "[SceneGraph]") {
	SceneGraph scene_graph;

	UID root_uid = 0;
	UID child1_uid = 1;
	UID child2_uid = 2;

	Ref<Node> root_node = create_ref<Node>();
	root_node->uid = root_uid;
	Ref<Node> child_node = create_ref<Node>();
	child_node->uid = child1_uid;
	Ref<Node> child_node2 = create_ref<Node>();
	child_node2->uid = child2_uid;

	// Test push_root
	scene_graph.push_node(root_node);
	REQUIRE(scene_graph.get_root()->children[0] == root_node);

	root_node->add_child(child_node);
	root_node->add_child(child_node2);

	// Test find_node
	REQUIRE(scene_graph.find_node(root_uid) == root_node.get());
	REQUIRE(scene_graph.find_node(child1_uid) == child_node.get());
	REQUIRE(scene_graph.find_node(child2_uid) == child_node2.get());
	REQUIRE(scene_graph.find_node(999) == nullptr); // Non-existent UID

	// Test remove_node
	REQUIRE(scene_graph.remove_node(child1_uid) == true);
	REQUIRE(scene_graph.find_node(child1_uid) == nullptr);
	REQUIRE(scene_graph.remove_node(999) == false); // Non-existent UID
}

TEST_CASE("SceneGraph Traversal", "[SceneGraph]") {
	SceneGraph scene_graph;

	UID root_uid = 0;
	UID child1_uid = 1;
	UID child2_uid = 2;

	Ref<Node> root_node = create_ref<Node>();
	root_node->uid = root_uid;
	Ref<Node> child_node = create_ref<Node>();
	child_node->uid = child1_uid;
	Ref<Node> child_node2 = create_ref<Node>();
	child_node2->uid = child2_uid;

	scene_graph.push_node(root_node);
	root_node->add_child(child_node);
	root_node->add_child(child_node2);

	// Test traverse
	bool found_child1 = false;
	scene_graph.traverse([&](Node* node) {
		if (node->uid == child1_uid) {
			found_child1 = true;
			return true; // Stop traversal
		}
		return false;
	});
	REQUIRE(found_child1);

	bool found_child2 = false;
	scene_graph.traverse([&](Node* node) {
		if (node->uid == child2_uid) {
			found_child2 = true;
			return true; // Stop traversal
		}
		return false;
	});
	REQUIRE(found_child2);
}
