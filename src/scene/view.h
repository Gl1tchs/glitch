#pragma once

#include "scene/component_lookup.h"
#include "scene/entity.h"

template <typename... TComponents> class SceneView {
public:
	SceneView(EntityContainer* p_entities) : entities(p_entities) {
		if (sizeof...(TComponents) == 0) {
			all = true;
		} else {
			// unpack the parameter list and set the component mask accordingly
			const uint32_t component_ids[] = {
				get_component_id<TComponents>()...
			};
			for (int i = 0; i < sizeof...(TComponents); i++) {
				component_mask.set(component_ids[i]);
			}
		}
	}

	class Iterator {
	public:
		Iterator(EntityContainer* p_entities, uint32_t p_index,
				ComponentMask p_mask, bool p_all) :
				entities(p_entities),
				index(p_index),
				mask(p_mask),
				all(p_all) {}

		Entity operator*() const { return entities->at(index).id; }

		bool operator==(const Iterator& p_other) const {
			return index == p_other.index || index == entities->size();
		}

		bool operator!=(const Iterator& p_other) const {
			return !(*this == p_other);
		}

		Iterator operator++() {
			do {
				index++;
			} while (index < entities->size() && !_is_index_valid());

			return *this;
		}

	private:
		bool _is_index_valid() {
			return
					// It's a valid entity ID
					is_entity_valid(entities->at(index).id) &&
					// It has the correct component mask
					(all || mask == (mask & entities->at(index).mask));
		}

	private:
		EntityContainer* entities;

		uint32_t index;
		ComponentMask mask;

		bool all = false;
	};

	const Iterator begin() const {
		uint32_t first_index = 0;
		while (first_index < entities->size() &&
				(component_mask !=
								(component_mask &
										entities->at(first_index).mask) ||
						!is_entity_valid(entities->at(first_index).id))) {
			first_index++;
		}

		return Iterator(entities, first_index, component_mask, all);
	}

	const Iterator end() const {
		return Iterator(entities, entities->size(), component_mask, all);
	}

private:
	EntityContainer* entities = nullptr;
	ComponentMask component_mask;
	bool all = false;
};
