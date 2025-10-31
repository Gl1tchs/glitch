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
	static Ref<Scene> get_scene();
	static void set_scene(Ref<Scene> p_scene);

	static void on_create();

	static void on_update(float p_dt);

	static void on_destroy();
};

} //namespace gl