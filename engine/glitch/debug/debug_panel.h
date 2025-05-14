/**
 * @file debug_panel.h
 *
 */

#pragma once

#include "glitch/scene/scene.h"

/**
 * An ImGui window responsible for scene manipulation
 */
class DebugPanel {
public:
	DebugPanel();

	void draw(Scene* p_scene);

private:
	bool show_panel = false;
};