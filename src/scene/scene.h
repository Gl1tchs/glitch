#pragma once

#include "scene/component_lookup.h"
#include "scene/entity.h"
#include "scene/view.h"

class GL_API Scene {
public:
	/**
	 * @brief create new entity instance on the scene
	 */
	Entity create();

	/**
	 * @brief find out wether the entity is valid or not
	 */
	bool is_valid(Entity p_entity);

	/**
	 * @brief removes entity from the scene and increments
	 * version
	 */
	void destroy(Entity p_entity);

	/**
	 * @brief assigns specified component to the entity
	 */
	template <typename T, typename... TArgs>
	T* assign(Entity p_entity, TArgs&&... args) {
		if (!is_valid(p_entity)) {
			return nullptr;
		}

		const uint32_t component_id = get_component_id<T>();

		if (component_pools.size() <= component_id) {
			component_pools.resize(component_id + 1, nullptr);
		}
		if (component_pools[component_id] == nullptr) {
			component_pools[component_id] = new ComponentPool(sizeof(T));
		}

		T* component = new (component_pools[component_id]->get(
				get_entity_index(p_entity))) T(std::forward<TArgs>(args)...);

		entities[get_entity_index(p_entity)].mask.set(component_id);

		return component;
	}

	/**
	 * @brief assigns specified components to the entity
	 */
	template <typename... TComponents>
		requires MultiParameter<TComponents...>
	std::tuple<TComponents*...> assign(Entity p_entity) {
		if (!is_valid(p_entity)) {
			return std::make_tuple(static_cast<TComponents*>(nullptr)...);
		}

		return std::make_tuple(assign<TComponents>(p_entity)...);
	}

	/**
	 * @brief remove specified component from the entity
	 */
	template <typename T> void remove(Entity p_entity) {
		if (!is_valid(p_entity)) {
			return;
		}

		const uint32_t component_id = get_component_id<T>();
		entities[get_entity_index(p_entity)].mask.reset(component_id);
	}

	/**
	 * @brief remove specified components from the entity
	 */
	template <typename... TComponents>
		requires MultiParameter<TComponents...>
	void remove(Entity p_entity) {
		if (!is_valid(p_entity)) {
			return;
		}

		(remove<TComponents>(p_entity), ...);
	}

	/**
	 * @brief get specified component from the entity
	 */
	template <typename T> T* get(Entity p_entity) {
		if (!is_valid(p_entity)) {
			return nullptr;
		}

		const uint32_t component_id = get_component_id<T>();
		if (!entities[get_entity_index(p_entity)].mask.test(component_id)) {
			return nullptr;
		}

		T* component = static_cast<T*>(
				component_pools[component_id]->get(get_entity_index(p_entity)));

		return component;
	}

	/**
	 * @brief get specified components from the entity
	 */
	template <typename... TComponents>
		requires MultiParameter<TComponents...>
	std::tuple<TComponents*...> get(Entity p_entity) {
		if (!is_valid(p_entity)) {
			return std::make_tuple(static_cast<TComponents*>(nullptr)...);
		}

		return std::make_tuple(get<TComponents>(p_entity)...);
	}

	/**
	 * @brief find out wether an entity has the specified components
	 */
	template <typename... TComponents> bool has(Entity p_entity) {
		if (!is_valid(p_entity)) {
			return false;
		}

		const uint32_t component_ids[] = { get_component_id<TComponents>()... };
		for (int i = 0; i < sizeof...(TComponents); i++) {
			if (!entities[get_entity_index(p_entity)].mask.test(
						component_ids[i])) {
				return false;
			}
		}

		return true;
	}

	/**
	 * @brief get entities with specified components,
	 * if any component not provided it will return all
	 * of the entities
	 */
	template <typename... TComponents> SceneView<TComponents...> view() {
		return SceneView<TComponents...>(&entities);
	}

private:
	uint32_t entity_counter = 0;

	EntityContainer entities;

	// indices of the `entities` list which are available
	std::queue<Entity> free_indices;

	std::vector<ComponentPool*> component_pools;
};
