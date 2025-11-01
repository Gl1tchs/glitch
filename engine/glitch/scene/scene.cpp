#include "glitch/scene/scene.h"

#include "glitch/scene/components.h"
#include "glitch/scene/entity.h"
#include "glitch/scripting/script_system.h"

namespace gl {

void Scene::start() {
	GL_PROFILE_SCOPE;

	running = true;

	ScriptSystem::on_runtime_start(this);

	for (Entity entity : view<ScriptComponent>()) {
		ScriptSystem::invoke_on_create();
	}
}

void Scene::update(float p_dt) {
	GL_PROFILE_SCOPE;

	if (paused && step_frames-- <= 0) {
		return;
	}

	ScriptSystem::invoke_on_update(p_dt);
}

void Scene::stop() {
	GL_PROFILE_SCOPE;

	running = false;

	ScriptSystem::invoke_on_destroy();

	ScriptSystem::on_runtime_stop();
}

void Scene::set_paused(bool p_paused) { paused = p_paused; }

void Scene::step(uint32_t frames) { step_frames = frames; }

bool Scene::is_running() const { return running; }

bool Scene::is_paused() const { return paused; }

void Scene::copy_to(Scene& p_dest) {
	Registry::copy_to(p_dest);

	p_dest.entity_map.clear();
	p_dest.entity_map.reserve(this->entity_map.size());

	// Copy entities
	for (const auto& [uid, entity] : this->entity_map) {
		p_dest.entity_map[uid] = Entity((EntityId)entity, &p_dest);
	}
}

Entity Scene::create(const std::string& p_name, UID p_parent_id) {
	return create(UID(), p_name, p_parent_id);
}

Entity Scene::create(UID p_uid, const std::string& p_name, UID p_parent_id) {
	Entity entity{ spawn(), this };

	entity.add_component<IdComponent>(p_uid, p_name);
	entity.add_component<Transform>();
	entity.add_component<RelationComponent>();

	if (p_parent_id) {
		Entity parent = find_by_id(p_parent_id);
		if (parent) {
			entity.set_parent(parent);
		}
	}

	entity_map[p_uid] = entity;

	return entity;
}

void Scene::destroy(Entity p_entity) {
	if (!p_entity.is_valid()) {
		return;
	}

	for (auto child : p_entity.get_children()) {
		destroy(child);
	}

	entity_map.erase(p_entity.get_uid());

	despawn(p_entity);
}

void Scene::destroy(UID p_uid) {
	Entity entity = find_by_id(p_uid);
	if (!entity) {
		return;
	}

	despawn(entity);
}

bool Scene::exists(UID p_uid) const {
	return entity_map.find(p_uid) != entity_map.end();
}

Entity Scene::find_by_id(UID p_uid) {
	if (entity_map.find(p_uid) != entity_map.end()) {
		return { entity_map.at(p_uid), this };
	}
	return {};
}

Entity Scene::find_by_name(const std::string& p_name) {
	const auto view = this->view<IdComponent>();
	const auto entity_it =
			std::find_if(view.begin(), view.end(), [&](const auto& entity) {
				return get<IdComponent>(entity)->tag == p_name;
			});

	if (entity_it != view.end()) {
		return { *entity_it, this };
	} else {
		return {}; // Return an empty entity if not found
	}
}

} //namespace gl