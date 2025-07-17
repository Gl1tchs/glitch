/**
 * @file render_queue.h
 *
 */

#pragma once

#include "glitch/renderer/mesh.h"

namespace gl {

struct RenderObject {
	glm::mat4 transform;
	Ref<MeshPrimitive> primitive;
};

class RenderQueue {
public:
	using Iter = std::vector<RenderObject>::iterator;
	using ConstIter = std::vector<RenderObject>::const_iterator;

	RenderQueue() = default;

	void add(const RenderObject& p_renderable);

	void sort();

	void clear();

	bool empty() const;

	Iter begin();
	Iter end();

	ConstIter begin() const;
	ConstIter end() const;

private:
	std::vector<RenderObject> renderables;
};

} //namespace gl