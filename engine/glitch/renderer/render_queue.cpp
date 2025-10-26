#include "glitch/renderer/render_queue.h"

namespace gl {

void RenderQueue::add(const RenderObject& p_renderable) {
	renderables.push_back(p_renderable);
}

void RenderQueue::sort() {
	std::sort(renderables.begin(), renderables.end(),
			[](const RenderObject& a, const RenderObject& b) {
				return std::tie(a.primitive->material->definition->pipeline,
							   a.transform[3].z) <
						std::tie(b.primitive->material->definition->pipeline,
								b.transform[3].z);
			});
}

void RenderQueue::clear() { renderables.clear(); }

bool RenderQueue::empty() const { return renderables.empty(); }

RenderQueue::Iter RenderQueue::begin() { return renderables.begin(); }
RenderQueue::Iter RenderQueue::end() { return renderables.end(); }

RenderQueue::ConstIter RenderQueue::begin() const {
	return renderables.begin();
}
RenderQueue::ConstIter RenderQueue::end() const { return renderables.end(); }

void RenderQueue::clear_light_sources() {
	directional_light = {};
	point_lights.clear();
	spotlights.clear();
}

} //namespace gl