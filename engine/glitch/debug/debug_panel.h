/**
 * @file debug_panel.h
 *
 */

#pragma once

/**
 * An ImGui window responsible for scene manipulation
 */
class DebugPanel {
public:
	DebugPanel();

	void draw();

private:
	bool show_panel = false;
};