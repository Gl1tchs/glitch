/**
 * @file render_queue.h
 *
 */

#pragma once

#include "glitch/renderer/light_sources.h"
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

	// Renderable API

	void add(const RenderObject& p_renderable);

	void sort();

	void clear();

	bool empty() const;

	Iter begin();
	Iter end();

	ConstIter begin() const;
	ConstIter end() const;

	// Lightning API

	template <LightSource S> std::vector<S> get_light_sources() const;

	template <LightSource S> void push_light_source(S p_light);

	template <LightSource S> void pop_light_source();

	template <LightSource S> void clear_light_source();

	void clear_light_sources();

private:
	std::vector<RenderObject> renderables;

	DirectionalLight directional_light;
	std::vector<PointLight> point_lights;
	std::vector<Spotlight> spotlights;
};

template <>
inline std::vector<DirectionalLight> RenderQueue::get_light_sources() const {
	return { directional_light };
}

template <>
inline void RenderQueue::push_light_source(DirectionalLight p_light) {
	directional_light = p_light;
}

template <> inline void RenderQueue::pop_light_source<DirectionalLight>() {
	directional_light = {};
}

template <> inline void RenderQueue::clear_light_source<DirectionalLight>() {
	directional_light = {};
}

template <>
inline std::vector<PointLight> RenderQueue::get_light_sources() const {
	return point_lights;
}

template <> inline void RenderQueue::push_light_source(PointLight p_light) {
	point_lights.push_back(p_light);
}

template <> inline void RenderQueue::pop_light_source<PointLight>() {
	point_lights.pop_back();
}

template <> inline void RenderQueue::clear_light_source<PointLight>() {
	point_lights.clear();
}

template <>
inline std::vector<Spotlight> RenderQueue::get_light_sources() const {
	return spotlights;
}

template <> inline void RenderQueue::push_light_source(Spotlight p_light) {
	spotlights.push_back(p_light);
}

template <> inline void RenderQueue::pop_light_source<Spotlight>() {
	spotlights.pop_back();
}

template <> inline void RenderQueue::clear_light_source<Spotlight>() {
	spotlights.clear();
}

} //namespace gl