#include "glitch/scene/scene.h"

#include "glitch/asset/asset_system.h"
#include "glitch/renderer/light_sources.h"
#include "glitch/scene/components.h"
#include "glitch/scene/entity.h"
#include "glitch/scene/gltf_loader.h"
#include "glitch/scripting/script_system.h"

#include <json/json.hpp>

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

Entity Scene::create(const std::string& p_name, Entity p_parent) {
	return create(UID(), p_name, p_parent);
}

Entity Scene::create(UID p_uid, const std::string& p_name, Entity p_parent) {
	Entity entity{ spawn(), this };

	entity.add_component<IdComponent>(p_uid, p_name);
	entity.add_component<Transform>();
	entity.add_component<RelationComponent>();

	if (p_parent) {
		entity.set_parent(p_parent);
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

static json _serialize_entity(const Entity& p_entity) {
	GL_ASSERT(p_entity.has_component<IdComponent>());
	GL_ASSERT(p_entity.has_component<Transform>());
	GL_ASSERT(p_entity.has_component<RelationComponent>());

	json j;

	j["id"] = p_entity.get_uid();
	j["tag"] = p_entity.get_name();
	j["parent_id"] = p_entity.get_relation().parent_id;
	j["transform"] = p_entity.get_transform();

	if (p_entity.has_component<GLTFSourceComponent>()) {
		j["gltf_source_component"] = *p_entity.get_component<GLTFSourceComponent>();
	}
	if (p_entity.has_component<GLTFInstanceComponent>()) {
		j["gltf_instance_component"] = *p_entity.get_component<GLTFInstanceComponent>();
	}

	if (p_entity.has_component<CameraComponent>()) {
		j["camera_component"] = *p_entity.get_component<CameraComponent>();
	}
	if (p_entity.has_component<DirectionalLight>()) {
		j["directional_light"] = *p_entity.get_component<DirectionalLight>();
	}
	if (p_entity.has_component<PointLight>()) {
		j["point_light"] = *p_entity.get_component<PointLight>();
	}
	if (p_entity.has_component<ScriptComponent>()) {
		j["script_component"] = *p_entity.get_component<ScriptComponent>();
	}

	return j;
}

bool Scene::serialize(std::string_view p_path, std::shared_ptr<Scene> p_scene) {
	const auto abs_path = AssetSystem::get_absolute_path(p_path);
	if (!abs_path) {
		GL_LOG_ERROR("[Scene::serialize] Unable to serialize scene to path: {}", p_path);
		return false;
	}

	GL_LOG_TRACE("[Scene::serialize] Serializing scene to: {}", p_path);

	json j;
	j["entities"] = nlohmann::json::array();

	for (Entity e : p_scene->view()) {
		j["entities"].push_back(_serialize_entity(e));
	}

	j["assets"] = json();
	AssetSystem::serialize(j["assets"]);

	std::ofstream file(abs_path.get_value());
	if (!file.is_open()) {
		GL_LOG_ERROR("[Scene::serialize] Unable to open file at path '{}' for serialization",
				abs_path.get_value().string());
		return false;
	}

	// Write serialized json to the file.
	file << j.dump(2);

	return true;
}

static Entity _deserialize_entity(const json& p_json, std::shared_ptr<Scene> p_scene) {
	UID id;
	if (p_json.contains("id")) {
		p_json.at("id").get_to(id);
	} else {
		GL_LOG_ERROR("[_deserialize_entity] Entity does not contain "
					 "'id' field"
					 "field. Stopping serialization.");
		return INVALID_ENTITY;
	}

	std::string tag;
	if (p_json.contains("tag")) {
		p_json.at("tag").get_to(tag);
	} else {
		GL_LOG_ERROR("[_deserialize_entity] Entity does not contain "
					 "'tag' field "
					 "field. Stopping serialization.");
		return INVALID_ENTITY;
	}

	// Remove unusued assets
	AssetSystem::collect_garbage();

	Entity entity = p_scene->create(id, tag);

	if (p_json.contains("parent_id")) {
		entity.get_component<RelationComponent>()->parent_id = p_json.at("parent_id").get<UID>();
	} else {
		entity.get_component<RelationComponent>()->parent_id = INVALID_UID;
	}

	if (p_json.contains("transform")) {
		p_json.at("transform").get_to(entity.get_transform());
	}

	if (p_json.contains("gltf_source_component")) {
		GLTFSourceComponent* gltf_sc = entity.add_component<GLTFSourceComponent>();
		p_json.at("gltf_source_component").get_to(*gltf_sc);
	}
	if (p_json.contains("gltf_instance_component")) {
		GLTFInstanceComponent* gltf_ic = entity.add_component<GLTFInstanceComponent>();
		p_json.at("gltf_instance_component").get_to(*gltf_ic);
	}

	if (p_json.contains("camera_component")) {
		entity.add_component<CameraComponent>();
		p_json.at("camera_component").get_to(*entity.get_component<CameraComponent>());
	}
	if (p_json.contains("directional_light")) {
		entity.add_component<DirectionalLight>();
		p_json.at("directional_light").get_to(*entity.get_component<DirectionalLight>());
	}
	if (p_json.contains("point_light")) {
		entity.add_component<PointLight>();
		p_json.at("point_light").get_to(*entity.get_component<PointLight>());
	}
	if (p_json.contains("script_component")) {
		entity.add_component<ScriptComponent>();
		p_json.at("script_component").get_to(*entity.get_component<ScriptComponent>());
	}

	return entity;
}

bool Scene::deserialize(std::string_view p_path, std::shared_ptr<Scene> p_scene) {
	const auto abs_path = AssetSystem::get_absolute_path(p_path);
	if (!abs_path) {
		GL_LOG_ERROR("[Scene::deserialize] Unable to deserialize scene from path: {}", p_path);
		return false;
	}

	GL_LOG_TRACE("[Scene::deserialize] Deserializing scene to: {}", p_path);

	json j;

	std::ifstream file(abs_path.get_value());
	if (!file.is_open()) {
		GL_LOG_ERROR("[Scene::deserialize] Unable to open file at path '{}' for deserialization",
				abs_path.get_value().string());
		return false;
	}

	file >> j;

	std::shared_ptr<Scene> new_scene = std::make_shared<Scene>();
	for (const json& j_entity : j["entities"]) {
		Entity _ = _deserialize_entity(j_entity, new_scene);
	}

	// Update entity / child hierarchy
	for (Entity e : new_scene->view()) {
		const auto& rc = e.get_relation();
		if (rc.parent_id) {
			std::optional<Entity> p = new_scene->find_by_id(rc.parent_id);
			if (!p) {
				continue;
			}

			p->get_relation().children_ids.push_back(e.get_uid());
			e.get_transform().parent = &p->get_transform();
		}
	}

	// Load GLTF Models and merge them
	for (const Entity& gltf_source : new_scene->view<GLTFSourceComponent>()) {
		const GLTFSourceComponent* gltf_sc = gltf_source.get_component<GLTFSourceComponent>();

		// Load the gltf model
		// TODO: make this multithreaded
		std::shared_ptr<Scene> gltf_scene = std::make_shared<Scene>();
		GLTFLoadError result = GLTFLoader::load(gltf_scene, gltf_sc->asset_path);
		if (result != GLTFLoadError::NONE) {
			GL_LOG_ERROR("[Scene::deserialize] Unable to load GLTF model from path ''",
					gltf_sc->asset_path);
			continue;
		}

		// Iterate through gltf instances
		for (Entity gltf_instance : new_scene->view<GLTFInstanceComponent>()) {
			const GLTFInstanceComponent* gltf_ic =
					gltf_instance.get_component<GLTFInstanceComponent>();

			// Skip if instance is not owned by this source
			if (gltf_ic->source_model_id != gltf_sc->model_id) {
				continue;
			}

			// Iterate through loaded instances
			for (const Entity& gltf_instance_loaded : gltf_scene->view<GLTFInstanceComponent>()) {
				const GLTFInstanceComponent* gltf_ic_loaded =
						gltf_instance_loaded.get_component<GLTFInstanceComponent>();

				// Skip the instance if node ids' do not match
				if (gltf_ic_loaded->gltf_node_id != gltf_ic->gltf_node_id) {
					continue;
				}

				// Copy the mesh component
				if (const MeshComponent* mc_loaded =
								gltf_instance_loaded.get_component<MeshComponent>()) {
					MeshComponent* mc = gltf_instance.add_component<MeshComponent>();
					mc->mesh = mc_loaded->mesh;
				}
			}
		}
	}

	new_scene->copy_to(*p_scene);

	return true;
}

} //namespace gl
