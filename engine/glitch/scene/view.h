/**
 * @file scene_view.h
 */

#pragma once

#include "glitch/scene/component_lookup.h"

namespace gl {

/**
 * Class who queries entities within the `Scene` that is also iterable
 */
template <typename... TComponents> class SceneView {
public:
	SceneView(EntityContainer* entities) : _entities(entities) {
		if constexpr (sizeof...(TComponents) == 0) {
			_all = true;
		} else {
			// unpack the parameter list and set the component mask accordingly
			const uint32_t component_ids[] = { get_component_id<TComponents>()... };
			for (int i = 0; i < sizeof...(TComponents); i++) {
				_component_mask.set(component_ids[i]);
			}
		}
	}

	class Iterator {
	public:
		Iterator(EntityContainer* entities, uint32_t index, ComponentMask mask, bool all) :
				_entities(entities), _index(index), _mask(mask), _all(all) {}

		EntityId operator*() const { return _entities->at(_index).id; }

		bool operator==(const Iterator& other) const {
			return _index == other._index || _index == _entities->size();
		}

		bool operator!=(const Iterator& p_other) const { return !(*this == p_other); }

		Iterator operator++() {
			do {
				_index++;
			} while (_index < _entities->size() && !_is_index_valid());

			return *this;
		}

	private:
		bool _is_index_valid() {
			return
					// It's a valid entity ID
					is_entity_valid(_entities->at(_index).id) &&
					// It has the correct component mask
					(_all || _mask == (_mask & _entities->at(_index).mask));
		}

	private:
		EntityContainer* _entities;

		uint32_t _index;
		ComponentMask _mask;

		bool _all = false;
	};

	const Iterator begin() const {
		uint32_t first_index = 0;
		while (first_index < _entities->size() &&
				(_component_mask != (_component_mask & _entities->at(first_index).mask) ||
						!is_entity_valid(_entities->at(first_index).id))) {
			first_index++;
		}

		return Iterator(_entities, first_index, _component_mask, _all);
	}

	const Iterator end() const {
		return Iterator(_entities, _entities->size(), _component_mask, _all);
	}

private:
	EntityContainer* _entities = nullptr;
	ComponentMask _component_mask;
	bool _all = false;
};

} //namespace gl