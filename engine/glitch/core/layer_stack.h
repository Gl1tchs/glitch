/**
 * @file layer_stack.h
 *
 */

#pragma once

#include "glitch/core/layer.h"

namespace gl {

class LayerStack {
public:
	using LayerContainer = std::vector<Layer*>;

	LayerStack() = default;

	~LayerStack() { clear(); }

	void clear() {
		for (auto it = layers.rbegin(); it != layers.rend(); it++) {
			(*it)->destroy();
			delete *it;
		}

		layers.clear();
	}

	template <typename T, typename... Args>
	void push_layer(Args&&... p_args)
		requires std::is_base_of_v<Layer, T>
	{
		Layer* layer = new T(std::forward<Args>(p_args)...);
		layer->start();
		layers.push_back(layer);
	}

	LayerContainer::iterator begin() { return layers.begin(); }
	LayerContainer::iterator end() { return layers.end(); }

	LayerContainer::const_iterator begin() const { return layers.begin(); }
	LayerContainer::const_iterator end() const { return layers.end(); }

	LayerContainer::reverse_iterator rbegin() { return layers.rbegin(); }
	LayerContainer::reverse_iterator rend() { return layers.rend(); }

	LayerContainer::const_reverse_iterator rbegin() const { return layers.rbegin(); }
	LayerContainer::const_reverse_iterator rend() const { return layers.rend(); }

private:
	LayerContainer layers;
};

} // namespace gl
