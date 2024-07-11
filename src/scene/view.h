#pragma once

#include "scene/scene.h"

template <typename... TComponents> class SceneViewIterator {
public:
	SceneViewIterator(Scene* p_scene, uint32_t p_index, ComponentMask p_mask,
			bool p_all) :
			scene(p_scene), index(p_index), mask(p_mask), all(p_all) {}

	Entity operator*() const { return scene->entities[index].id; }

	bool operator==(const SceneViewIterator& p_other) const {
		return index == p_other.index || index == scene->entities.size();
	}

	bool operator!=(const SceneViewIterator& p_other) const {
		return !(*this == p_other);
	}

	SceneViewIterator operator++() {
		do {
			index++;
		} while (index < scene->entities.size() && !_is_index_valid());

		return *this;
	}

private:
	bool _is_index_valid() {
		return
				// It's a valid entity ID
				is_entity_valid(scene->entities[index].id) &&
				// It has the correct component mask
				(all || mask == (mask & scene->entities[index].mask));
	}

private:
	Scene* scene;

	uint32_t index;
	ComponentMask mask;

	bool all = false;
};

template <typename... TComponents> class SceneView {
public:
	SceneView(Scene& p_scene) : scene(&p_scene) {
		if (sizeof...(TComponents) == 0) {
			all = true;
		} else {
			// unpack the parameter list and set the component mask accordingly
			uint32_t component_ids[] = { 0,
				get_component_id<TComponents>()... };
			for (int i = 1; i < (sizeof...(TComponents) + 1); i++) {
				component_mask.set(component_ids[i]);
			}
		}
	}

	const SceneViewIterator<TComponents...> begin() const {
		uint32_t first_index = 0;
		while (first_index < scene->entities.size() &&
				(component_mask !=
								(component_mask &
										scene->entities[first_index].mask) ||
						!is_entity_valid(scene->entities[first_index].id))) {
			first_index++;
		}

		return SceneViewIterator<TComponents...>(
				scene, first_index, component_mask, all);
	}

	const SceneViewIterator<TComponents...> end() const {
		return SceneViewIterator<TComponents...>(
				scene, scene->entities.size(), component_mask, all);
	}

private:
	Scene* scene = nullptr;
	ComponentMask component_mask;
	bool all = false;
};
