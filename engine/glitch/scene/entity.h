/**
 * @file entity.h
 *
 */

#pragma once

#include "glitch/core/transform.h"
#include "glitch/core/uid.h"
#include "glitch/scene/component_lookup.h"

namespace gl {

class Scene;

struct IdComponent {
	UID id;
	std::string tag;
};

struct RelationComponent {
	UID parent_id = INVALID_UID;
	std::vector<UID> children_ids = {};
};

/**
 * EntityId + Scene wrapper
 */
class GL_API Entity {
public:
	constexpr Entity() : _handle(INVALID_ENTITY_ID) {}

	Entity(EntityId handle, Scene* scene) : _handle(handle), _scene(scene) {}

	Entity(const Entity& other) = default;

	template <typename T, typename... Args> T* add_component(Args&&... args);

	template <typename T> T* get_component();

	template <typename T> const T* get_component() const;

	template <typename TComponent> bool has_component() const;

	template <typename T> void remove_component();

	RelationComponent& get_relation();
	const RelationComponent& get_relation() const;

	std::optional<Entity> get_parent() const;
	void set_parent(Entity parent);

	bool is_parent() const;

	bool is_child() const;

	std::vector<Entity> get_children() const;

	std::optional<Entity> find_child_by_id(UID uid) const;
	std::optional<Entity> find_child_by_name(const std::string& name) const;

	bool remove_child(Entity child);

	static bool is_parent_of(Entity parent, Entity child);

	const UID& get_uid() const;

	const std::string& get_name() const;
	void set_name(const std::string& name);

	Transform& get_transform();
	const Transform& get_transform() const;

	bool is_valid() const;

	operator bool() const;

	operator EntityId() const;

	operator uint32_t() const;

	bool operator==(const Entity& other) const;

	bool operator!=(const Entity& other) const;

private:
	EntityId _handle;
	Scene* _scene = nullptr;

	friend class HierarchyPanel;
};

constexpr Entity INVALID_ENTITY = {};

} //namespace gl
