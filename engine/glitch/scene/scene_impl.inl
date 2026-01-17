#pragma once

#include "glitch/scene/scene.h"
#include "glitch/scene/view.h"

namespace gl {

template <typename... TComponents> class EntityView {
public:
	EntityView(SceneView<TComponents...> id_view, Scene* scene) :
			_id_view(id_view), _scene(scene) {}

	class Iterator {
	public:
		Iterator(typename SceneView<TComponents...>::Iterator id_iter, Scene* scene) :
				_id_iter(id_iter), _scene(scene) {}

		Entity operator*() const { return Entity(*_id_iter, _scene); }

		bool operator!=(const Iterator& other) const { return _id_iter != other._id_iter; }
		bool operator==(const Iterator& other) const { return _id_iter == other._id_iter; }

		Iterator& operator++() {
			++_id_iter;
			return *this;
		}

	private:
		Scene* _scene;

		typename SceneView<TComponents...>::Iterator _id_iter;
	};

	Iterator begin() const { return Iterator(_id_view.begin(), _scene); }
	Iterator end() const { return Iterator(_id_view.end(), _scene); }

private:
	Scene* _scene;

	SceneView<TComponents...> _id_view;
};

template <typename... TComponents> EntityView<TComponents...> Scene::view() {
	auto id_view = Registry::view<TComponents...>();
	return EntityView<TComponents...>(id_view, this);
}

template <typename T, typename... Args> inline T* Entity::add_component(Args&&... args) {
	return _scene->assign<T>(_handle, std::forward<Args>(args)...);
}

template <typename T> inline T* Entity::get_component() { return _scene->get<T>(_handle); }

template <typename T> inline const T* Entity::get_component() const {
	return _scene->get<T>(_handle);
}

template <typename TComponent> inline bool Entity::has_component() const {
	return _scene->has<TComponent>(_handle);
}

template <typename T> inline void Entity::remove_component() { _scene->remove<T>(_handle); }

} //namespace gl