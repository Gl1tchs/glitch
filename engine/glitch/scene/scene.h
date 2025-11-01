/**
 * @file scene.h
 *
 */

#pragma once

#include "glitch/core/uid.h"
#include "glitch/scene/entity.h"
#include "glitch/scene/registry.h"

namespace gl {

template <typename... TComponents> class EntityView;

/**
 * Registry wrapper using `Entity` type and relations.
 */
class GL_API Scene : public Registry {
public:
	Scene() = default;
	virtual ~Scene() = default;

	void copy_to(Scene& p_dest);

	// Scene logic

	void start();

	void update(float p_dt);

	void stop();

	void set_paused(bool p_paused);

	void step(uint32_t p_frames = 1);

	bool is_running() const;

	bool is_paused() const;

	// ECS

	Entity create(const std::string& p_name, UID p_parent_id = 0);
	Entity create(UID p_uid, const std::string& p_name,
			UID p_parent_id = INVALID_UID);

	void destroy(Entity p_entity);
	void destroy(UID p_uid);

	bool exists(UID p_uid) const;

	Entity find_by_id(UID p_uid);
	Entity find_by_name(const std::string& p_name);

	/**
	 * Get entities with specified components,
	 * returning an iterable view of `Entity` objects.
	 */
	template <typename... TComponents> EntityView<TComponents...> view();

private:
	std::unordered_map<UID, Entity> entity_map;

	bool running = false;
	bool paused = false;
	int step_frames = 0;
};

} //namespace gl

#include "glitch/scene/scene_impl.inl"