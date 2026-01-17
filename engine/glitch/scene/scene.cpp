#include "glitch/scene/scene.h"

#include "glitch/asset/asset_system.h"
#include "glitch/core/debug/profiling.h"
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

	_running = true;

	ScriptSystem::on_runtime_start(this);

	ScriptSystem::invoke_on_create();
}

void Scene::update(float dt) {
	GL_PROFILE_SCOPE;

	if (_paused && _step_frames-- <= 0) {
		return;
	}

	ScriptSystem::invoke_on_update(dt);
}

void Scene::stop() {
	GL_PROFILE_SCOPE;

	_running = false;

	ScriptSystem::invoke_on_destroy();

	ScriptSystem::on_runtime_stop();
}

void Scene::set_paused(bool paused) { _paused = paused; }

void Scene::step(uint32_t frames) { _step_frames = frames; }

bool Scene::is_running() const { return _running; }

bool Scene::is_paused() const { return _paused; }

void Scene::copy_to(Scene& dest) {
	Registry::copy_to(dest);

	dest._entity_map.clear();

	// Copy entities
	dest._entity_map.reserve(this->_entity_map.size());
	std::transform(this->_entity_map.begin(), this->_entity_map.end(),
			std::inserter(dest._entity_map, dest._entity_map.end()), [&dest](const auto& pair) {
				const auto& [uid, entity] = pair;
				return std::make_pair(uid, Entity(static_cast<EntityId>(entity), &dest));
			});

	// Update entity transforms
	// NOTE: this must do in a seperate loop to ensure all entities are copied
	for (Entity entity : dest.view<Transform>()) {
		if (entity.is_child()) {
			Entity parent = *entity.get_parent();
			entity.get_transform().parent = &parent.get_transform();
		}
	}
}

Entity Scene::create(const std::string& name, Entity parent) { return create(UID(), name, parent); }

Entity Scene::create(UID uid, const std::string& name, Entity parent) {
	Entity entity{ spawn(), this };

	entity.add_component<IdComponent>(uid, name);
	entity.add_component<Transform>();
	entity.add_component<RelationComponent>();

	if (parent) {
		entity.set_parent(parent);
	}

	_entity_map[uid] = entity;

	return entity;
}

void Scene::destroy(Entity entity) {
	if (!entity.is_valid()) {
		return;
	}

	// Release asset handles for GC
	// TODO do this dynamically.
	if (auto mc = entity.get_component<MeshComponent>()) {
		mc->mesh.release();
	}
	if (auto mc = entity.get_component<MaterialComponent>()) {
		mc->handle.release();
	}

	// Destroy the children if any
	for (auto child : entity.get_children()) {
		destroy(child);
	}

	// If `entity` is a child of some other entity then reset relation
	if (std::optional<Entity> parent = entity.get_parent()) {
		std::vector<UID>& parent_children = parent->get_relation().children_ids;
		parent_children.erase(
				std::find(parent_children.begin(), parent_children.end(), entity.get_uid()));
	}

	_entity_map.erase(entity.get_uid());

	despawn(entity);
}

void Scene::destroy(UID uid) {
	std::optional<Entity> entity = find_by_id(uid);
	if (!entity) {
		return;
	}

	despawn(*entity);
}

bool Scene::exists(UID uid) const { return _entity_map.find(uid) != _entity_map.end(); }

std::optional<Entity> Scene::find_by_id(UID uid) {
	const auto it = _entity_map.find(uid);
	if (it == _entity_map.end()) {
		return {};
	}
	return it->second;
}

std::optional<Entity> Scene::find_by_name(const std::string& name) {
	const auto it = std::find_if(
			_entity_map.begin(), _entity_map.end(), [&](const std::pair<UID, Entity>& entity_pair) {
				return entity_pair.second.get_name() == name;
			});
	if (it == _entity_map.end()) {
		return {};
	}

	return it->second;
}

static json _serialize_entity(const Entity& entity) {
	GL_ASSERT(entity.has_component<IdComponent>());
	GL_ASSERT(entity.has_component<Transform>());
	GL_ASSERT(entity.has_component<RelationComponent>());

	json j;

	j["id"] = entity.get_uid();
	j["tag"] = entity.get_name();
	j["parent_id"] = entity.get_relation().parent_id;
	j["transform"] = entity.get_transform();

	if (const GLTFSourceComponent* gltf_sc = entity.get_component<GLTFSourceComponent>()) {
		j["gltf_source_component"] = *gltf_sc;
	}
	if (const GLTFInstanceComponent* gltf_ic = entity.get_component<GLTFInstanceComponent>()) {
		j["gltf_instance_component"] = *gltf_ic;
	}

	if (const MaterialComponent* mc = entity.get_component<MaterialComponent>()) {
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
							entity.get_name(), uniform.name);
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
					entity.get_name());
		}
	}

	if (const CameraComponent* cc = entity.get_component<CameraComponent>()) {
		j["camera_component"] = *cc;
	}
	if (const DirectionalLight* dl = entity.get_component<DirectionalLight>()) {
		j["directional_light"] = *dl;
	}
	if (const PointLight* pl = entity.get_component<PointLight>()) {
		j["point_light"] = *pl;
	}
	if (const Script* sc = entity.get_component<Script>()) {
		j["script"] = *sc;
	}

	return j;
}

bool Scene::serialize(std::string_view path, std::shared_ptr<Scene> scene) {
	const auto abs_path = AssetSystem::get_absolute_path(path);
	if (!abs_path) {
		GL_LOG_ERROR("[Scene::serialize] Unable to serialize scene to path: {}", path);
		return false;
	}

	GL_LOG_TRACE("[Scene::serialize] Serializing scene to: {}", path);

	json j;
	j["entities"] = nlohmann::json::array();

	for (Entity e : scene->view()) {
		j["entities"].push_back(_serialize_entity(e));
	}

	j["assets"] = json();
	AssetSystem::serialize(j["assets"]);

	// Write serialized json to the file.
	if (json_save(path, j) != JSONLoadError::NONE) {
		GL_LOG_ERROR("[Scene::serialize] Unable to open file at path '{}' for serialization",
				abs_path.value().string());
		return false;
	}

	return true;
}

static Entity _deserialize_entity(const json& json, std::shared_ptr<Scene> scene) {
	UID id;
	if (json.contains("id")) {
		json.at("id").get_to(id);
	} else {
		GL_LOG_ERROR("[_deserialize_entity] Entity does not contain "
					 "'id' field"
					 "field. Stopping serialization.");
		return INVALID_ENTITY;
	}

	std::string tag;
	if (json.contains("tag")) {
		json.at("tag").get_to(tag);
	} else {
		GL_LOG_ERROR("[_deserialize_entity] Entity does not contain "
					 "'tag' field "
					 "field. Stopping serialization.");
		return INVALID_ENTITY;
	}

	Entity entity = scene->create(id, tag);

	if (json.contains("parent_id")) {
		entity.get_component<RelationComponent>()->parent_id = json.at("parent_id").get<UID>();
	} else {
		entity.get_component<RelationComponent>()->parent_id = INVALID_UID;
	}

	if (json.contains("transform")) {
		json.at("transform").get_to(entity.get_transform());
	}

	if (json.contains("gltf_source_component")) {
		GLTFSourceComponent* gltf_sc = entity.add_component<GLTFSourceComponent>();
		json.at("gltf_source_component").get_to(*gltf_sc);
	}
	if (json.contains("gltf_instance_component")) {
		GLTFInstanceComponent* gltf_ic = entity.add_component<GLTFInstanceComponent>();
		json.at("gltf_instance_component").get_to(*gltf_ic);
	}

	if (json.contains("material_component")) {
		MaterialComponent* mc = entity.add_component<MaterialComponent>();
		mc->handle = INVALID_ASSET_HANDLE;

		json["material_component"]["definition_path"].get_to(mc->definition_path);

		// Deserialize uniforms if any
		if (json["material_component"].contains("uniforms") &&
				json["material_component"]["uniforms"].is_array()) {
			for (const auto& uniform : json["material_component"]["uniforms"]) {
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
							value = uniform["value"].get<Vec2f>();
							break;
						case ShaderUniformVariableType::VEC3:
							value = uniform["value"].get<Vec3f>();
							break;
						case ShaderUniformVariableType::VEC4:
							value = uniform["value"].get<Vec4f>();
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

	if (json.contains("camera_component")) {
		CameraComponent* cc = entity.add_component<CameraComponent>();
		json.at("camera_component").get_to(*cc);
	}
	if (json.contains("directional_light")) {
		DirectionalLight* dl = entity.add_component<DirectionalLight>();
		json.at("directional_light").get_to(*dl);
	}
	if (json.contains("point_light")) {
		PointLight* pl = entity.add_component<PointLight>();
		json.at("point_light").get_to(*pl);
	}
	if (json.contains("script")) {
		Script* sc = entity.add_component<Script>();
		json.at("script").get_to(*sc);
	}

	return entity;
}

bool Scene::deserialize(std::string_view path, std::shared_ptr<Scene> scene) {
	const auto res = json_load(path);
	if (!res) {
		GL_LOG_ERROR(
				"[Scene::deserialize] Unable to open file at path '{}' for deserialization", path);
		return false;
	}

	const json& j = res.value();

	if (!j.contains("entities") || !j["entities"].is_array()) {
		GL_LOG_ERROR("[Scene::deserialize] Unable to deserialize scene from path '{}', invalid "
					 "entity list.",
				path);
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

	new_scene->copy_to(*scene);

	return true;
}

} //namespace gl
