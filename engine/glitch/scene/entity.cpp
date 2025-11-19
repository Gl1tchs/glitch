#include "glitch/scene/entity.h"

#include "glitch/scene/scene.h"

namespace gl {

RelationComponent& Entity::get_relation() { return *get_component<RelationComponent>(); }

const RelationComponent& Entity::get_relation() const {
	return *get_component<RelationComponent>();
}

std::optional<Entity> Entity::get_parent() const {
	const UID parent_id = get_relation().parent_id;
	if (!parent_id) {
		return {};
	}

	return scene->find_by_id(parent_id);
}

void Entity::set_parent(Entity parent) {
	// check if this is not the parent of "parent"
	if (is_parent_of(*this, parent)) {
		return;
	}

	// Remove old parent if exists.
	std::optional<Entity> current_parent = get_parent();
	if (current_parent) {
		(*current_parent).remove_child(*this);
	}

	// If an invalid entity provided then carry this into top levels
	if (!parent) {
		return;
	}

	RelationComponent& parent_relation = parent.get_relation();

	// Setup relations between components
	RelationComponent& relation = get_relation();
	relation.parent_id = parent.get_uid();
	parent_relation.children_ids.push_back(get_uid());

	// Make transform values relative to the new parent.
	get_transform().parent = &parent.get_transform();
}

bool Entity::is_parent() const { return get_relation().children_ids.size() > 0; }

bool Entity::is_child() const { return get_relation().parent_id != INVALID_UID; }

std::vector<Entity> Entity::get_children() const {
	if (!has_component<RelationComponent>()) {
		return {};
	}

	const std::vector<UID>& children_ids = get_relation().children_ids;

	std::vector<Entity> children;
	children.reserve(children_ids.size());

	std::transform(children_ids.begin(), children_ids.end(), std::back_inserter(children),
			[&](const auto& child_id) {
				Entity entity = scene->find_by_id(child_id).value();
				return entity;
			});

	// remove entities that are not found
	children.erase(std::remove_if(children.begin(), children.end(),
						   [](const Entity& entity) { return !entity; }),
			children.end());

	return children;
}

std::optional<Entity> Entity::find_child_by_id(UID p_uid) const {
	const auto& children = get_relation().children_ids;
	const auto it = std::find(children.begin(), children.end(), p_uid);
	if (it == children.end()) {
		return std::nullopt;
	}

	return scene->find_by_id(*it);
}

std::optional<Entity> Entity::find_child_by_name(const std::string& p_name) const {
	std::optional<Entity> child_to_find = scene->find_by_name(p_name);
	if (!child_to_find) {
		return std::nullopt;
	}

	std::optional<Entity> child_parent = child_to_find->get_parent();
	if (!child_parent || (uint32_t)*child_parent != handle) {
		return std::nullopt;
	}

	return *child_to_find;
}

bool Entity::remove_child(Entity child) {
	auto& children_ids = get_relation().children_ids;

	const auto it = std::find(children_ids.begin(), children_ids.end(), child.get_uid());

	if (it != children_ids.end()) {
		// Set local positions as the world position
		Transform& child_transform = child.get_transform();
		child_transform.local_position = child_transform.get_position();
		child_transform.local_rotation = child_transform.get_rotation();
		child_transform.local_scale = child_transform.get_scale();

		child_transform.parent = nullptr;

		// move child to top level
		child.get_relation().parent_id = INVALID_UID;
		children_ids.erase(it);

		return true;
	}

	return false;
}

bool Entity::is_parent_of(Entity parent, Entity child) {
	if (!child) {
		return false;
	}

	// check recursively if child is one of parent's
	std::optional<Entity> child_parent = child.get_parent();

	// if top level return false
	if (!child_parent) {
		return false;
	}
	// if parents are same then we found it
	else if (child_parent == parent) {
		return true;
	}
	// otherwise call the function again
	else {
		return is_parent_of(parent, *child_parent);
	}
}

const UID& Entity::get_uid() const { return get_component<IdComponent>()->id; }

const std::string& Entity::get_name() const { return get_component<IdComponent>()->tag; }

void Entity::set_name(const std::string& p_name) { get_component<IdComponent>()->tag = p_name; }

Transform& Entity::get_transform() { return *get_component<Transform>(); }

const Transform& Entity::get_transform() const { return *get_component<Transform>(); }

bool Entity::is_valid() const { return is_entity_valid(handle) && scene != nullptr; }

Entity::operator bool() const { return is_valid(); }

Entity::operator EntityId() const { return handle; }

Entity::operator uint32_t() const { return (uint32_t)handle; }

bool Entity::operator==(const Entity& other) const {
	return handle == other.handle && scene == other.scene;
}

bool Entity::operator!=(const Entity& other) const { return !(*this == other); }

} //namespace gl
