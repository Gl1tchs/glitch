/**
 * @file debug_panel.h
 *
 */

#pragma once

#include "glitch/scene_graph/scene_graph.h"

/**
 * An ImGui window responsible for scene manipulation
 */
class GL_API DebugPanel {
public:
	static void draw(const Ref<SceneNode>& p_graph_root);
};