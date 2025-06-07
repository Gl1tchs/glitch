/**
 * @file drawing_context.h
 *
 */

#pragma once

#include "glitch/renderer/camera.h"
#include "glitch/renderer/scene_graph.h"

struct DrawingContext {
	SceneGraph* scene_graph;

	PerspectiveCamera camera;
	Transform camera_transform;

	// Other rendering options, decals, debug lines etc...
};