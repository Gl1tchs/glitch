#include "glitch/scripting/script_system.h"

#include "glitch/scene/components.h"

namespace gl {

static std::mutex s_scene_mutex;
static Ref<Scene> s_scene = nullptr;

Ref<Scene> ScriptSystem::get_scene() { return s_scene; }

void ScriptSystem::set_scene(Ref<Scene> p_scene) { s_scene = p_scene; }

void ScriptSystem::on_create() {
	std::scoped_lock<std::mutex> lock(s_scene_mutex);

	if (!s_scene) {
		GL_LOG_ERROR("[ScriptSystem::on_create] No scene bound to traverse.");
		return;
	}

	for (Entity entity : s_scene->view<ScriptComponent>()) {
		ScriptComponent* sc = entity.get_component<ScriptComponent>();

		if (!sc->is_loaded) {
			switch (sc->load()) {
				case ScriptResult::LOAD_ERROR:
				case ScriptResult::INVALID_SCRIPT_FILE:
					GL_LOG_ERROR("Unable to load script file for Entity: {}",
							entity.get_uid().value);
					break;
				case ScriptResult::INVALID_TABLE:
					GL_LOG_ERROR("Script does not return a table in Entity: {}",
							entity.get_uid().value);
					break;
				default:
					break;
			}

			continue;
		}

		const ScriptResult result = ScriptEngine::exec_function(
				sc->script, "on_create", entity.get_uid().value);
		if (result == ScriptResult::FUNCTION_NOT_FOUND) {
			GL_LOG_WARNING("Script for entity {} has no 'on_create' function.",
					entity.get_uid().value);
		}
	}
}

void ScriptSystem::on_update(float p_dt) {
	std::scoped_lock<std::mutex> lock(s_scene_mutex);

	if (!s_scene) {
		GL_LOG_ERROR("[ScriptSystem::on_update] No scene bound to traverse.");
		return;
	}

	for (Entity entity : s_scene->view<ScriptComponent>()) {
		ScriptComponent* sc = entity.get_component<ScriptComponent>();

		const ScriptResult result = ScriptEngine::exec_function(
				sc->script, "on_update", entity.get_uid().value, p_dt);

		if (result == ScriptResult::FUNCTION_NOT_FOUND) {
			GL_LOG_WARNING("Script for entity {} has no 'on_update' function.",
					entity.get_uid().value);
		}
	}
}

void ScriptSystem::on_destroy() {
	std::scoped_lock<std::mutex> lock(s_scene_mutex);

	if (!s_scene) {
		GL_LOG_ERROR("[ScriptSystem::on_destroy] No scene bound to traverse.");
		return;
	}

	for (Entity entity : s_scene->view<ScriptComponent>()) {
		ScriptComponent* sc = entity.get_component<ScriptComponent>();

		const ScriptResult result = ScriptEngine::exec_function(
				sc->script, "on_destroy", entity.get_uid().value);

		if (result == ScriptResult::FUNCTION_NOT_FOUND) {
			GL_LOG_WARNING("Script for entity {} has no 'on_destroy' function.",
					entity.get_uid().value);
		}

		// Reset script metadata
		sc->reset();
	}
}

} //namespace gl