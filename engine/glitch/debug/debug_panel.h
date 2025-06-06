/**
 * @file debug_panel.h
 *
 */

#pragma once

#include "glitch/renderer/scene_graph.h"

/**
 * An ImGui window responsible for scene manipulation
 */
class DebugPanel {
public:
	static void draw(const Ref<SceneNode>& p_graph_root);
};