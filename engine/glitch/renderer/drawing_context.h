/**
 * @file drawing_context.h
 *
 */

#pragma once

#include "glitch/renderer/camera.h"
#include "glitch/renderer/scene_graph.h"

struct DrawingContext {
	Ref<SceneNode> root;

	PerspectiveCamera camera;
	Transform camera_transform;

	/**
	 * Traverses, filters and preprocess scene graph nodes that contributes to
	 * rendering
	 */
	void assign_scene_graph(SceneGraph p_graph);

	// Other rendering options, decals, debug lines etc...
};