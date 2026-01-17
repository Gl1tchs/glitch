/**
 * @file scene.h
 *
 */

#pragma once

#include "glitch/core/uid.h"
#include "glitch/scene/entity.h"
#include "glitch/scene/registry.h"

#include <unordered_map>

namespace gl {

template <typename... TComponents> class EntityView;

/**
 * Registry wrapper using `Entity` type and relations.
 */
class GL_API Scene : public Registry {
public:
	Scene() = default;
	virtual ~Scene() = default;

	void copy_to(Scene& dest);

	// Scene logic

	void start();

	void update(float dt);

	void stop();

	void set_paused(bool paused);

	void step(uint32_t frames = 1);

	bool is_running() const;

	bool is_paused() const;

	// ECS

	Entity create(const std::string& name, Entity parent = INVALID_ENTITY);
	Entity create(UID uid, const std::string& name, Entity parent = INVALID_ENTITY);

	void destroy(Entity entity);
	void destroy(UID uid);

	bool exists(UID uid) const;

	std::optional<Entity> find_by_id(UID uid);
	std::optional<Entity> find_by_name(const std::string& name);

	/**
	 * Get entities with specified components,
	 * returning an iterable view of `Entity` objects.
	 */
	template <typename... TComponents> EntityView<TComponents...> view();

	static bool serialize(std::string_view path, const std::shared_ptr<Scene> scene);
	static bool deserialize(std::string_view path, std::shared_ptr<Scene> scene);

private:
	std::unordered_map<UID, Entity> _entity_map;

	bool _running = false;
	bool _paused = false;
	int _step_frames = 0;
};

} //namespace gl

#include "glitch/scene/scene_impl.inl"