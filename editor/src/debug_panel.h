/**
 * @file debug_panel.h
 *
 */

#pragma once

#include <glitch/scene_graph/scene_graph.h>

using namespace gl;

class DebugPanel {
public:
	static void draw(const Ref<SceneNode>& p_graph_root);
};
