#include "glitch/scene/scene.h"

#include "glitch/asset/asset_system.h"
#include "glitch/renderer/light_sources.h"
#include "glitch/renderer/material.h"
#include "glitch/renderer/texture.h"
#include "glitch/scene/components.h"
#include "glitch/scene/entity.h"
#include "glitch/scene/gltf_loader.h"
#include "glitch/scripting/script.h"
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

	// Release asset handles for GC
	// TODO do this dynamically.
	if (auto mc = p_entity.get_component<MeshComponent>()) {
		mc->mesh.release();
	}
	if (auto mc = p_entity.get_component<MaterialComponent>()) {
		mc->handle.release();
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

	if (const GLTFSourceComponent* gltf_sc = p_entity.get_component<GLTFSourceComponent>()) {
		j["gltf_source_component"] = *gltf_sc;
	}
	if (const GLTFInstanceComponent* gltf_ic = p_entity.get_component<GLTFInstanceComponent>()) {
		j["gltf_instance_component"] = *gltf_ic;
	}

	if (const MaterialComponent* mc = p_entity.get_component<MaterialComponent>()) {
		const auto material = AssetSystem::get<Material>(mc->handle);
		if (material) {
			j["material_component"]["definition_path"] = mc->definition_path;
			// Serialize uniforms
			j["material_component"]["uniforms"] = json::array();
			for (const auto& uniform : material->get_uniforms()) {
				const auto value = material->get_param(uniform.name);
				if (!value) {
					GL_LOG_WARNING(
							"[_serialize_entity] Unable to serialize MaterialComponent for entity "
							"'{}. Uniform field '{}' does not have a value.",
							p_entity.get_name(), uniform.name);
					continue;
				}

				// Skip if texture is a memory asset
				if (uniform.type == ShaderUniformVariableType::TEXTURE) {
					const auto& handle = std::get<AssetHandle>(*value);
					if (const auto meta = AssetSystem::get_metadata<Texture>(handle);
							meta && meta->is_memory_asset()) {
						continue;
					}
				}

				json uniform_json;
				uniform_json["name"] = uniform.name;
				uniform_json["binding"] = uniform.binding;
				uniform_json["type"] = uniform.type;
				std::visit([&](auto&& arg) { uniform_json["value"] = arg; }, *value);

				j["material_component"]["uniforms"].push_back(uniform_json);
			}
		} else {
			GL_LOG_WARNING("[_serialize_entity] Unable to serialize MaterialComponent for entity "
						   "'{}. Material metadata does not exist.",
					p_entity.get_name());
		}
	}

	if (const CameraComponent* cc = p_entity.get_component<CameraComponent>()) {
		j["camera_component"] = *cc;
	}
	if (const DirectionalLight* dl = p_entity.get_component<DirectionalLight>()) {
		j["directional_light"] = *dl;
	}
	if (const PointLight* pl = p_entity.get_component<PointLight>()) {
		j["point_light"] = *pl;
	}
	if (const Script* sc = p_entity.get_component<Script>()) {
		j["script"] = *sc;
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

	// Write serialized json to the file.
	if (json_save(p_path, j) != JSONLoadError::NONE) {
		GL_LOG_ERROR("[Scene::serialize] Unable to open file at path '{}' for serialization",
				abs_path.get_value().string());
		return false;
	}

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

	if (p_json.contains("material_component")) {
		MaterialComponent* mc = entity.add_component<MaterialComponent>();
		mc->handle = INVALID_ASSET_HANDLE;

		p_json["material_component"]["definition_path"].get_to(mc->definition_path);

		// Deserialize uniforms if any
		if (p_json["material_component"].contains("uniforms") &&
				p_json["material_component"]["uniforms"].is_array()) {
			for (const auto& uniform : p_json["material_component"]["uniforms"]) {
				if (!uniform.contains("name") || !uniform.contains("type") ||
						!uniform.contains("value")) {
					GL_LOG_WARNING("[_deserialize_entity] Unable to deserialize material for "
								   "entity '{}' uniform {}.",
							entity.get_name(),
							uniform.contains("name") ? uniform["name"].get<std::string>() : "");
					continue;
				}

				const std::string name = uniform["name"].get<std::string>();

				try {
					ShaderUniformVariable value;
					switch (uniform["type"].get<ShaderUniformVariableType>()) {
						case ShaderUniformVariableType::INT:
							value = uniform["value"].get<int>();
							break;
						case ShaderUniformVariableType::FLOAT:
							value = uniform["value"].get<float>();
							break;
						case ShaderUniformVariableType::VEC2:
							value = uniform["value"].get<glm::vec2>();
							break;
						case ShaderUniformVariableType::VEC3:
							value = uniform["value"].get<glm::vec3>();
							break;
						case ShaderUniformVariableType::VEC4:
							value = uniform["value"].get<glm::vec4>();
							break;
						case ShaderUniformVariableType::TEXTURE:
							value = uniform["value"].get<AssetHandle>();
							break;
					}

					mc->uniforms[name] = value;
				} catch (const json::exception&) {
					GL_LOG_ERROR("[_deserialize_entity] Unable to parse uniform value '{}' for "
								 "entity '{}'",
							name, entity.get_name());
				}
			}
		}
	}

	if (p_json.contains("camera_component")) {
		CameraComponent* cc = entity.add_component<CameraComponent>();
		p_json.at("camera_component").get_to(*cc);
	}
	if (p_json.contains("directional_light")) {
		DirectionalLight* dl = entity.add_component<DirectionalLight>();
		p_json.at("directional_light").get_to(*dl);
	}
	if (p_json.contains("point_light")) {
		PointLight* pl = entity.add_component<PointLight>();
		p_json.at("point_light").get_to(*pl);
	}
	if (p_json.contains("script")) {
		Script* sc = entity.add_component<Script>();
		p_json.at("script").get_to(*sc);
	}

	return entity;
}

bool Scene::deserialize(std::string_view p_path, std::shared_ptr<Scene> p_scene) {
	const auto res = json_load(p_path);
	if (!res) {
		GL_LOG_ERROR("[Scene::deserialize] Unable to open file at path '{}' for deserialization",
				p_path);
		return false;
	}

	const json& j = res.get_value();

	if (!j.contains("entities") || !j["entities"].is_array()) {
		GL_LOG_ERROR("[Scene::deserialize] Unable to deserialize scene from path '{}', invalid "
					 "entity list.",
				p_path);
		return false;
	}

	if (j.contains("assets") && j["assets"].is_object()) {
		AssetSystem::deserialize(j["assets"]);
	} else {
		GL_LOG_WARNING("[Scene::deserialize] Unable to deserialize asset registry.");
	}

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
	for (const Entity& source : new_scene->view<GLTFSourceComponent>()) {
		const GLTFSourceComponent* sc = source.get_component<GLTFSourceComponent>();

		// Load the gltf model
		// TODO: make this multithreaded
		std::shared_ptr<Scene> gltf_scene = std::make_shared<Scene>();
		if (GLTFLoader::load(gltf_scene, sc->asset_path) != GLTFLoadError::NONE) {
			GL_LOG_ERROR(
					"[Scene::deserialize] Unable to load GLTF model from path ''", sc->asset_path);
			continue;
		}

		// Iterate through gltf instances
		for (Entity instance : new_scene->view<GLTFInstanceComponent>()) {
			const GLTFInstanceComponent* ic = instance.get_component<GLTFInstanceComponent>();

			// Skip if instance is not owned by this source
			if (ic->source_model_id != sc->model_id) {
				continue;
			}

			// Iterate through loaded instances
			for (Entity gltf_entity : gltf_scene->view<GLTFInstanceComponent>()) {
				const GLTFInstanceComponent* gltf_ic_loaded =
						gltf_entity.get_component<GLTFInstanceComponent>();

				// Skip the instance if node ids' do not match
				if (gltf_ic_loaded->gltf_node_id != ic->gltf_node_id) {
					continue;
				}

				// Copy the mesh component
				if (MeshComponent const* gltf_mesh = gltf_entity.get_component<MeshComponent>()) {
					// Ensure we get the component if it exists (from deserialize) or add it
					MeshComponent* mc = instance.has_component<MeshComponent>()
							? instance.get_component<MeshComponent>()
							: instance.add_component<MeshComponent>();
					mc->mesh = gltf_mesh->mesh;
				}

				if (MaterialComponent* gltf_mc = gltf_entity.get_component<MaterialComponent>()) {
					if (MaterialComponent* instance_mc =
									instance.get_component<MaterialComponent>()) {
						// If definitions are same do not create new component but update
						// GLTF one.
						if (instance_mc->definition_path == gltf_mc->definition_path) {
							auto mat = AssetSystem::get<Material>(gltf_mc->handle);
							if (mat) {
							} else {
								GL_LOG_ERROR("[Scene::deserialize] Unable to retrieve material "
											 "from GLTF model for entity '{}'.",
										instance.get_name());
							}

							instance_mc->handle = std::move(gltf_mc->handle);

							// Update uniforms
							for (const auto& [name, uniform] : instance_mc->uniforms) {
								// This is now also this Entity's material
								if (!mat->set_param(name, uniform)) {
									GL_LOG_ERROR("[Scene::deserialize] Unable to set uniform "
												 "parameter '{}' "
												 "for definition '{}' for entity '{}'.",
											name, instance_mc->definition_path,
											instance.get_name());
								}
							}
						} else {
							// If definitions differ, initialize our custom material.
							if (auto handle = AssetSystem::create<Material>(
										instance_mc->definition_path)) {
								instance_mc->handle = std::move(*handle);

								const auto mat = AssetSystem::get<Material>(instance_mc->handle);

								// Update uniforms
								for (const auto& [name, uniform] : instance_mc->uniforms) {
									if (!mat->set_param(name, uniform)) {
										GL_LOG_ERROR("[Scene::deserialize] Unable to set uniform "
													 "parameter '{}' "
													 "for definition '{}' for entity '{}'.",
												name, instance_mc->definition_path,
												instance.get_name());
									}
								}
							} else {
								GL_LOG_ERROR("[Scene::deserialize] Unable to initialize material "
											 "from definition '{}' for entity '{}'.",
										instance_mc->definition_path, instance.get_name());
							}
						}
					} else {
						// CASE: No serialized material. Use GLTF defaults exactly.
						MaterialComponent* mc = instance.add_component<MaterialComponent>();
						mc->handle = std::move(gltf_mc->handle);
						mc->definition_path = std::move(gltf_mc->definition_path);
						mc->uniforms = std::move(gltf_mc->uniforms);

						// Copy parameter state from the source GLTF entity to the new instance
						const auto entity_mat = AssetSystem::get<Material>(mc->handle);
						const auto gltf_mat = AssetSystem::get<Material>(gltf_mc->handle);

						if (entity_mat && gltf_mat) {
							for (const auto& uniform : gltf_mat->get_uniforms()) {
								if (const auto value = gltf_mat->get_param(uniform.name)) {
									entity_mat->set_param(uniform.name, *value);
								}
							}
						}
					}
				}
			}
		}
	}

	new_scene->copy_to(*p_scene);

	return true;
}

} //namespace gl
