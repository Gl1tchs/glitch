#pragma once

inline constexpr uint32_t MAX_ENTITIES = 1000; // TODO: dynamically allocate
inline constexpr uint32_t MAX_COMPONENTS = 32;

// first 32 bits is index and last 32 bits are version
typedef uint64_t Entity;

constexpr inline Entity create_entity_id(uint32_t p_index, uint32_t p_version) {
	return ((Entity)p_index << 32) | p_version;
}

constexpr inline uint32_t get_entity_index(Entity p_entity) {
	return p_entity >> 32;
}

constexpr inline uint32_t get_entity_version(Entity p_entity) {
	// this conversation will loose the top 32 bits
	return (uint32_t)p_entity;
}

constexpr inline bool is_entity_valid(Entity p_entity) {
	return (p_entity >> 32) != -1;
}

inline constexpr Entity INVALID_ENTITY = create_entity_id(UINT32_MAX, 0);

typedef std::bitset<MAX_COMPONENTS> ComponentMask;

extern uint32_t s_component_counter;

// returns different id for different component types
template <class T> inline uint32_t get_component_id() {
	static uint32_t s_component_id = s_component_counter++;
	return s_component_id;
}

class ComponentPool {
public:
	ComponentPool(size_t p_element_size) : element_size(p_element_size) {
		data = new uint8_t[element_size * MAX_ENTITIES];
	}

	~ComponentPool() { delete[] data; }

	void* get(size_t p_index) { return data + p_index * element_size; }

private:
	uint8_t* data = nullptr;
	size_t element_size = 0;
};

class Scene {
public:
	Entity create();

	bool is_valid(Entity p_entity);

	void destroy(Entity p_entity);

	template <typename T> T* assign(Entity p_entity);

	template <typename T> void remove(Entity p_entity);

	template <typename T> T* get(Entity p_entity);

	template <typename T> bool has(Entity p_entity);

private:
	uint32_t entity_counter = 0;

	struct EntityDesc {
		Entity id;
		ComponentMask mask;
	};
	std::vector<EntityDesc> entities;

	// indices of the `entities` list which are available
	std::queue<Entity> free_indices;

	std::vector<ComponentPool*> component_pools;

	template <typename... TComponents> friend class SceneView;
	template <typename... TComponents> friend class SceneViewIterator;
};

template <typename T> T* Scene::assign(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return nullptr;
	}

	const uint32_t component_id = get_component_id<T>();

	if (component_pools.size() <= component_id) {
		component_pools.resize(component_id + 1, nullptr);
	}
	if (component_pools[component_id] == nullptr) {
		component_pools[component_id] = new ComponentPool(sizeof(T));
	}

	T* component = new (
			component_pools[component_id]->get(get_entity_index(p_entity))) T();

	entities[get_entity_index(p_entity)].mask.set(component_id);

	return component;
}

template <typename T> void Scene::remove(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return;
	}

	const uint32_t component_id = get_component_id<T>();
	entities[get_entity_index(p_entity)].mask.reset(component_id);
}

template <typename T> T* Scene::get(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return nullptr;
	}

	const uint32_t component_id = get_component_id<T>();
	if (!entities[get_entity_index(p_entity)].mask.test(component_id)) {
		return nullptr;
	}

	T* component = static_cast<T*>(
			component_pools[component_id]->get(get_entity_index(p_entity)));

	return component;
}

template <typename T> bool Scene::has(Entity p_entity) {
	if (!is_valid(p_entity)) {
		return false;
	}

	const uint32_t component_id = get_component_id<T>();
	return entities[get_entity_index(p_entity)].mask.test(component_id);
}
