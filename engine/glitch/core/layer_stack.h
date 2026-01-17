/**
 * @file layer_stack.h
 *
 */

#pragma once

#include "glitch/core/layer.h"

#include <vector>

namespace gl {

class LayerStack {
public:
	using LayerContainer = std::vector<Layer*>;

	LayerStack() = default;

	~LayerStack() { clear(); }

	void clear() {
		for (auto it = _layers.rbegin(); it != _layers.rend(); it++) {
			(*it)->destroy();
			delete *it;
		}

		_layers.clear();
	}

	template <typename T, typename... Args>
	void push_layer(Args&&... args)
		requires std::is_base_of_v<Layer, T>
	{
		Layer* layer = new T(std::forward<Args>(args)...);
		layer->start();
		_layers.push_back(layer);
	}

	LayerContainer::iterator begin() { return _layers.begin(); }
	LayerContainer::iterator end() { return _layers.end(); }

	LayerContainer::const_iterator begin() const { return _layers.begin(); }
	LayerContainer::const_iterator end() const { return _layers.end(); }

	LayerContainer::reverse_iterator rbegin() { return _layers.rbegin(); }
	LayerContainer::reverse_iterator rend() { return _layers.rend(); }

	LayerContainer::const_reverse_iterator rbegin() const { return _layers.rbegin(); }
	LayerContainer::const_reverse_iterator rend() const { return _layers.rend(); }

private:
	LayerContainer _layers;
};

} // namespace gl
