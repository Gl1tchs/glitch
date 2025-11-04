#include "glitch/scene/scene.h"

#include "glitch/scene/entity.h"
#include "glitch/scripting/script_system.h"

namespace gl {

void Scene::start() {
	GL_PROFILE_SCOPE;

	running = true;

	ScriptSystem::on_runtime_start(this);

	ScriptSystem::invoke_on_create();
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

	// Copy entities
	p_dest.entity_map.reserve(this->entity_map.size());
	std::transform(this->entity_map.begin(), this->entity_map.end(),
			std::inserter(p_dest.entity_map, p_dest.entity_map.end()), [&p_dest](const auto& pair) {
				const auto& [uid, entity] = pair;
				return std::make_pair(uid, Entity(static_cast<EntityId>(entity), &p_dest));
			});

	// Update entity transforms
	// NOTE: this must do in a seperate loop to ensure all entities are copied
	for (Entity entity : p_dest.view<Transform>()) {
		if (entity.is_child()) {
			Entity parent = *entity.get_parent();
			entity.get_transform().parent = &parent.get_transform();
		}
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
		std::optional<Entity> parent = find_by_id(p_parent_id);
		if (parent) {
			entity.set_parent(*parent);
		}
	}

	entity_map[p_uid] = entity;

	return entity;
}

void Scene::destroy(Entity p_entity) {
	if (!p_entity.is_valid()) {
		return;
	}

	// Destroy the children if any
	for (auto child : p_entity.get_children()) {
		destroy(child);
	}

	// If `p_entity` is a child of some other entity then reset relation
	if (std::optional<Entity> parent = p_entity.get_parent()) {
		std::vector<UID>& parent_children = parent->get_relation().children_ids;
		parent_children.erase(
				std::find(parent_children.begin(), parent_children.end(), p_entity.get_uid()));
	}

	entity_map.erase(p_entity.get_uid());

	despawn(p_entity);
}

void Scene::destroy(UID p_uid) {
	std::optional<Entity> entity = find_by_id(p_uid);
	if (!entity) {
		return;
	}

	despawn(*entity);
}

bool Scene::exists(UID p_uid) const { return entity_map.find(p_uid) != entity_map.end(); }

std::optional<Entity> Scene::find_by_id(UID p_uid) {
	const auto it = entity_map.find(p_uid);
	if (it == entity_map.end()) {
		return {};
	}
	return it->second;
}

std::optional<Entity> Scene::find_by_name(const std::string& p_name) {
	const auto it = std::find_if(
			entity_map.begin(), entity_map.end(), [&](const std::pair<UID, Entity>& entity_pair) {
				return entity_pair.second.get_name() == p_name;
			});
	if (it == entity_map.end()) {
		return {};
	}

	return it->second;
}

} //namespace gl