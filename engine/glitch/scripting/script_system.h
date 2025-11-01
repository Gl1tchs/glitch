/**
 * @file script_system.h
 *
 */

#pragma once

#include "glitch/scene/scene.h"

namespace gl {

/**
 * Script system that is responsible for loading and executing
 * scripts.
 */
class GL_API ScriptSystem {
public:
	static Scene* get_scene();

	static bool is_running();

	static void on_runtime_start(Scene* p_scene);
	static void on_runtime_stop();

	static void invoke_on_create();

	static void invoke_on_update(float p_dt);

	static void invoke_on_destroy();
};

} //namespace gl