/**
 * @file registry.h
 */

#pragma once

#include "glitch/core/templates/concepts.h"
#include "glitch/scene/component_lookup.h"
#include "glitch/scene/view.h"

namespace gl {

/**
 * Container of entities and components assigned to them.
 */
class GL_API Registry {
public:
	~Registry();

	void clear();

	void copy_to(Registry& p_dest);

	/**
	 * Create new entity instance on the scene
	 */
	EntityId spawn();

	/**
	 * Find out wether the entity is valid or not
	 */
	bool is_valid(EntityId p_entity);

	/**
	 * Removes entity from the scene and increments
	 * version
	 */
	void despawn(EntityId p_entity);

	/**
	 * Assigns specified component to the entity
	 */
	template <typename T, typename... TArgs> T* assign(EntityId p_entity, TArgs&&... args) {
		if (!is_valid(p_entity)) {
			return nullptr;
		}

		const uint32_t component_id = get_component_id<T>();

		if (component_pools.size() <= component_id) {
			component_pools.resize(component_id + 1, nullptr);
			pool_helpers.resize(component_id + 1);
		}
		if (component_pools[component_id] == nullptr) {
			component_pools[component_id] = new ComponentPool(sizeof(T));
			pool_helpers[component_id] = PoolHelpers{
				.element_size = sizeof(T),
				// Copy function (uses placement new + copy constructor)
				.copy_fn = [](void* dest,
								   const void* src) { new (dest) T(*static_cast<const T*>(src)); },
				// Destroy function (calls destructor)
				.destroy_fn = [](void* data) { static_cast<T*>(data)->~T(); },
			};
		}

		// Call destructor if component already exists
		if (entities[get_entity_index(p_entity)].mask.test(component_id)) {
			pool_helpers[component_id].destroy_fn(
					component_pools[component_id]->get(get_entity_index(p_entity)));
		}

		T* component = new (component_pools[component_id]->get(get_entity_index(p_entity)))
				T(std::forward<TArgs>(args)...);

		entities[get_entity_index(p_entity)].mask.set(component_id);

		return component;
	}

	/**
	 * Assigns specified components to the entity
	 */
	template <typename... TComponents>
		requires MultiParameter<TComponents...>
	std::tuple<TComponents*...> assign(EntityId p_entity) {
		if (!is_valid(p_entity)) {
			return std::make_tuple(static_cast<TComponents*>(nullptr)...);
		}

		return std::make_tuple(assign<TComponents>(p_entity)...);
	}

	/**
	 * Remove specified component from the entity
	 */
	template <typename T> void remove(EntityId p_entity) {
		if (!is_valid(p_entity)) {
			return;
		}

		const uint32_t component_id = get_component_id<T>();
		const uint32_t entity_idx = get_entity_index(p_entity);

		if (entities[entity_idx].mask.test(component_id)) {
			// call component's destructor
			if (pool_helpers.size() > component_id && pool_helpers[component_id].destroy_fn) {
				pool_helpers[component_id].destroy_fn(
						component_pools[component_id]->get(entity_idx));
			}
			entities[entity_idx].mask.reset(component_id);
		}
	}

	/**
	 * Remove specified components from the entity
	 */
	template <typename... TComponents>
		requires MultiParameter<TComponents...>
	void remove(EntityId p_entity) {
		if (!is_valid(p_entity)) {
			return;
		}

		(remove<TComponents>(p_entity), ...);
	}

	/**
	 * Get specified component from the entity
	 */
	template <typename T> T* get(EntityId p_entity) {
		if (!is_valid(p_entity)) {
			return nullptr;
		}

		const uint32_t component_id = get_component_id<T>();
		if (!entities[get_entity_index(p_entity)].mask.test(component_id)) {
			return nullptr;
		}

		T* component =
				static_cast<T*>(component_pools[component_id]->get(get_entity_index(p_entity)));

		return component;
	}

	/**
	 * Get specified components from the entity
	 */
	template <typename... TComponents>
		requires MultiParameter<TComponents...>
	std::tuple<TComponents*...> get(EntityId p_entity) {
		if (!is_valid(p_entity)) {
			return std::make_tuple(static_cast<TComponents*>(nullptr)...);
		}

		return std::make_tuple(get<TComponents>(p_entity)...);
	}

	/**
	 * Find out wether an entity has the specified components
	 */
	template <typename... TComponents> bool has(EntityId p_entity) {
		if (!is_valid(p_entity)) {
			return false;
		}

		const uint32_t component_ids[] = { get_component_id<TComponents>()... };
		for (int i = 0; i < sizeof...(TComponents); i++) {
			if (!entities[get_entity_index(p_entity)].mask.test(component_ids[i])) {
				return false;
			}
		}

		return true;
	}

	/**
	 * Get entities with specified components,
	 * if no component provided it will return all
	 * of the entities
	 */
	template <typename... TComponents> SceneView<TComponents...> view() {
		return SceneView<TComponents...>(&entities);
	}

private:
	struct PoolHelpers {
		size_t element_size = 0;
		void (*copy_fn)(void*, const void*) = nullptr;
		void (*destroy_fn)(void*) = nullptr;
	};

	uint32_t entity_counter = 0;
	EntityContainer entities;
	std::queue<EntityId> free_indices;
	std::vector<ComponentPool*> component_pools;
	// parallel vector to component_pools for component destruction logic
	std::vector<PoolHelpers> pool_helpers;
};

} //namespace gl