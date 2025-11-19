#pragma once

#include "glitch/scene/scene.h"
#include "glitch/scene/view.h"

namespace gl {

template <typename... TComponents> class EntityView {
public:
	EntityView(SceneView<TComponents...> id_view, Scene* p_scene) :
			id_view(id_view), scene(p_scene) {}

	class Iterator {
	public:
		Iterator(typename SceneView<TComponents...>::Iterator id_iter, Scene* p_scene) :
				id_iter(id_iter), scene(p_scene) {}

		Entity operator*() const { return Entity(*id_iter, scene); }

		bool operator!=(const Iterator& other) const { return id_iter != other.id_iter; }
		bool operator==(const Iterator& other) const { return id_iter == other.id_iter; }

		Iterator& operator++() {
			++id_iter;
			return *this;
		}

	private:
		Scene* scene;

		typename SceneView<TComponents...>::Iterator id_iter;
	};

	Iterator begin() const { return Iterator(id_view.begin(), scene); }
	Iterator end() const { return Iterator(id_view.end(), scene); }

private:
	Scene* scene;

	SceneView<TComponents...> id_view;
};

template <typename... TComponents> EntityView<TComponents...> Scene::view() {
	auto id_view = Registry::view<TComponents...>();
	return EntityView<TComponents...>(id_view, this);
}

template <typename T, typename... Args> inline T* Entity::add_component(Args&&... p_args) {
	return scene->assign<T>(handle, std::forward<Args>(p_args)...);
}

template <typename T> inline T* Entity::get_component() { return scene->get<T>(handle); }

template <typename T> inline const T* Entity::get_component() const {
	return scene->get<T>(handle);
}

template <typename TComponent> inline bool Entity::has_component() const {
	return scene->has<TComponent>(handle);
}

template <typename T> inline void Entity::remove_component() { scene->remove<T>(handle); }

} //namespace gl