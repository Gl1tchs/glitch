/**
 * @file entity.h
 *
 */

#pragma once

#include "glitch/core/transform.h"
#include "glitch/core/uid.h"
#include "glitch/scene/scene.h"

struct IdComponent {
	UID id;
	std::string tag;
};

struct RelationComponent {
	UID parent_id = 0;
	std::vector<UID> children_ids = {};
};

/**
 * EntityId + Scene wrapper
 */
class GL_API Entity {
public:
	constexpr Entity() : handle(INVALID_ENTITY_ID) {}

	Entity(EntityId p_handle, Scene* p_scene) :
			handle(p_handle), scene(p_scene) {}

	Entity(const Entity& p_other) = default;

	template <typename T, typename... Args>
	auto* add_component(Args&&... p_args) {
		return scene->assign<T>(handle, std::forward<Args>(p_args)...);
	}

	template <typename T> T* get_component() { return scene->get<T>(handle); }

	template <typename T> const T* get_component() const {
		return scene->get<T>(handle);
	}

	template <typename... Components> bool has_component() const {
		return scene->has<Components...>(handle);
	}

	template <typename T> void remove_component() { scene->remove<T>(handle); }

	RelationComponent* get_relation();
	const RelationComponent* get_relation() const;

	Entity get_parent() const;
	void set_parent(Entity p_parent);

	bool is_parent() const;

	bool is_child() const;

	std::vector<Entity> get_children() const;

	bool remove_child(Entity p_child);

	static bool is_parent_of(Entity p_parent, Entity p_child);

	const UID& get_uid() const;

	const std::string& get_name();

	Transform* get_transform();

	operator bool() const;

	operator EntityId() const;

	operator uint32_t() const;

	bool operator==(const Entity& p_other) const;

	bool operator!=(const Entity& p_other) const;

private:
	EntityId handle;
	Scene* scene = nullptr;

	friend class HierarchyPanel;
};

constexpr Entity INVALID_ENTITY = {};
