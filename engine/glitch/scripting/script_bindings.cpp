#include "glitch/core/application.h"
#include "glitch/core/event/input.h"
#include "glitch/core/transform.h"
#include "glitch/scene/scene.h"
#include "glitch/scripting/script_system.h"

#include <lua.hpp>

#include "ffi_lua_resource.gen.h"

namespace gl {

std::mutex g_script_mutex;

extern "C" {

typedef uint32_t EntityUID;

// --- Global functions ---

static void debug_log(const char* message);

// --- Entity API  ---

static EntityUID entity_create(const char* name);
static void entity_destroy(EntityUID self);
static int entity_is_valid(EntityUID self);
static EntityUID entity_find_by_id(EntityUID p_id);
static EntityUID entity_find_by_name(const char* p_name);
static const char* entity_get_name(EntityUID self);
static void entity_set_name(EntityUID self, const char* name);
static EntityUID entity_get_parent(EntityUID self);
static void entity_set_parent(EntityUID self, EntityUID parent);
static EntityUID entity_find_child_by_id(EntityUID self, EntityUID id);
static EntityUID entity_find_child_by_name(EntityUID self, const char* name);
static Transform* entity_get_transform(EntityUID self);

// --- Transform API ---

static void transform_rotate(Transform* transform, float angle, glm::vec3 axis);
static glm::vec3 transform_get_forward(Transform* transform);
static glm::vec3 transform_get_right(Transform* transform);
static glm::vec3 transform_get_up(Transform* transform);

// --- Input API ---

static int input_get_key_down(int key_code);
static int input_get_key_up(int key_code);
static int input_get_mouse_down(int mouse_code);
static int input_get_mouse_up(int mouse_code);

// --- Window API ---

static void window_set_title(const char* title);
static int window_set_cursor_mode();
static void Window_SetCursorMode(int mode);
static glm::vec2 window_set_size();

} // extern "C"

static void run_string(lua_State* L, const char* p_code) {
	if (luaL_dostring(L, p_code) != LUA_OK) {
		GL_LOG_ERROR("[LUA] FFI Bind Error: {}", lua_tostring(L, -1));
		lua_pop(L, 1);
		GL_ASSERT(false); // FFI scripts must compile
	}
}

static void run_string(lua_State* L, const std::string& p_code) { run_string(L, p_code.c_str()); }

static void bind_function(
		lua_State* L, const char* p_func_path, const char* p_func_def, uintptr_t p_fnptr) {
	run_string(L, std::format("{} = ffi.cast('{}', {}ULL)", p_func_path, p_func_def, p_fnptr));
}

void register_ffi_bindings(lua_State* L) {
	// Load and run the FFI script
	run_string(L, lua_bindings::ffi_source);

	/* ---------------- Debug Namespace ---------------- */
	bind_function(
			L, "Engine.C_Functions.debug_log", "void (*)(const char*)", (uintptr_t)&debug_log);

	/* ---------------- Entity API ---------------- */
	bind_function(L, "Engine.C_Functions.entity_create", "uint32_t (*)(const char*)",
			(uintptr_t)&entity_create);
	bind_function(L, "Engine.C_Functions.entity_destroy", "void (*)(uint32_t)",
			(uintptr_t)&entity_destroy);
	bind_function(L, "Engine.C_Functions.entity_is_valid", "int (*)(uint32_t)",
			(uintptr_t)&entity_is_valid);
	bind_function(L, "Engine.C_Functions.entity_find_by_id", "uint32_t (*)(uint32_t)",
			(uintptr_t)&entity_find_by_id);
	bind_function(L, "Engine.C_Functions.entity_find_by_name", "uint32_t (*)(const char*)",
			(uintptr_t)&entity_find_by_name);
	bind_function(L, "Engine.C_Functions.entity_get_name", "const char* (*)(uint32_t)",
			(uintptr_t)&entity_get_name);
	bind_function(L, "Engine.C_Functions.entity_set_name", "void (*)(uint32_t, const char*)",
			(uintptr_t)&entity_set_name);
	bind_function(L, "Engine.C_Functions.entity_get_parent", "uint32_t (*)(uint32_t)",
			(uintptr_t)&entity_get_parent);
	bind_function(L, "Engine.C_Functions.entity_set_parent", "void (*)(uint32_t, uint32_t)",
			(uintptr_t)&entity_set_parent);
	bind_function(L, "Engine.C_Functions.entity_find_child_by_id",
			"uint32_t (*)(uint32_t, uint32_t)", (uintptr_t)&entity_find_child_by_id);
	bind_function(L, "Engine.C_Functions.entity_find_child_by_name",
			"uint32_t (*)(uint32_t, const char*)", (uintptr_t)&entity_find_child_by_name);
	bind_function(L, "Engine.C_Functions.entity_get_transform", "Transform* (*)(uint32_t)",
			(uintptr_t)&entity_get_transform);

	/* ---------------- Transform API ---------------- */
	bind_function(L, "Engine.C_Functions.transform_rotate", "void (*)(Transform*, float, Vec3)",
			(uintptr_t)&transform_rotate);
	bind_function(L, "Engine.C_Functions.transform_get_forward", "Vec3 (*)(Transform*)",
			(uintptr_t)&transform_get_forward);
	bind_function(L, "Engine.C_Functions.transform_get_right", "Vec3 (*)(Transform*)",
			(uintptr_t)&transform_get_right);
	bind_function(L, "Engine.C_Functions.transform_get_up", "Vec3 (*)(Transform*)",
			(uintptr_t)&transform_get_up);

	/* ---------------- Input API ---------------- */
	bind_function(L, "Engine.C_Functions.input_get_key_down", "int (*)(int)",
			(uintptr_t)&input_get_key_down);
	bind_function(
			L, "Engine.C_Functions.input_get_key_up", "int (*)(int)", (uintptr_t)&input_get_key_up);
	bind_function(L, "Engine.C_Functions.input_get_mouse_down", "int (*)(int)",
			(uintptr_t)&input_get_mouse_down);
	bind_function(L, "Engine.C_Functions.input_get_mouse_up", "int (*)(int)",
			(uintptr_t)&input_get_mouse_up);

	/* ---------------- Window API ---------------- */
	bind_function(L, "Engine.C_Functions.window_set_title", "void (*)(const char*)",
			(uintptr_t)&window_set_title);
	bind_function(L, "Engine.C_Functions.window_set_cursor_mode", "int (*)()",
			(uintptr_t)&window_set_cursor_mode);
	bind_function(L, "Engine.C_Functions.Window_SetCursorMode", "void (*)(int)",
			(uintptr_t)&Window_SetCursorMode);
	bind_function(
			L, "Engine.C_Functions.window_set_size", "Vec2 (*)()", (uintptr_t)&window_set_size);
}

extern "C" {

void debug_log(const char* message) { GL_LOG_INFO("[LUA] {}", message); }

EntityUID entity_find_by_id(EntityUID p_id) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_find_by_id] No Scene bound to the ScriptSystem.");
		return 0;
	}

	std::optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(p_id);
	return entity ? p_id : 0;
}

EntityUID entity_find_by_name(const char* p_name) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_find_by_name] No Scene bound to the ScriptSystem.");
		return 0;
	}

	std::optional<Entity> entity = ScriptSystem::get_scene()->find_by_name(p_name);
	return entity ? (*entity).get_uid().value : 0;
}

Transform* entity_get_transform(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running() || self == 0) {
		GL_LOG_ERROR("[LUA] [entity_get_transform] No Scene bound to the ScriptSystem.");
		return nullptr;
	}

	std::optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (entity) {
		return &(*entity).get_transform();
	}
	return nullptr;
}

EntityUID entity_create(const char* name) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_create] No Scene bound to the ScriptSystem.");
		return 0;
	}

	Entity entity = ScriptSystem::get_scene()->create(name);
	return entity.get_uid().value;
}

void entity_destroy(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_destroy] No Scene bound to the ScriptSystem.");
		return;
	}

	Scene* scene = ScriptSystem::get_scene();
	std::optional<Entity> entity = scene->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[LUA] [entity_destroy] Unable to destroy Entity of Id: {}", self);
		return;
	}

	scene->destroy(*entity);
}

int entity_is_valid(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_is_valid] No Scene bound to the ScriptSystem.");
		return false;
	}

	std::optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	return entity && entity->is_valid();
}

const char* entity_get_name(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_get_name] No Scene bound to the ScriptSystem.");
		return "";
	}

	std::optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[LUA] [entity_get_name] Entity of Id: {} not found.", self);
		return "";
	}

	return entity->get_name().c_str();
}

void entity_set_name(EntityUID self, const char* name) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_set_name] No Scene bound to the ScriptSystem.");
		return;
	}

	std::optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[LUA] [entity_set_name] Entity of Id: {} not found.", self);
		return;
	}

	entity->set_name(name);
}

EntityUID entity_get_parent(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_get_parent] No Scene bound to the ScriptSystem.");
		return 0;
	}

	std::optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[LUA] [entity_get_parent] Entity of Id: {} not found.", self);
		return 0;
	}

	if (std::optional<Entity> parent = entity->get_parent()) {
		return parent->get_uid().value;
	}

	return 0;
}

void entity_set_parent(EntityUID self, EntityUID parent_id) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_set_parent] No Scene bound to the ScriptSystem.");
		return;
	}

	std::optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[LUA] [entity_set_parent] Entity of Id: {} not found.", self);
		return;
	}

	std::optional<Entity> parent = ScriptSystem::get_scene()->find_by_id(parent_id);
	if (!parent) {
		GL_LOG_ERROR("[LUA] [entity_set_parent] Parent of Id: {} not found.", self);
		return;
	}

	entity->set_parent(*parent);
}

EntityUID entity_find_child_by_id(EntityUID self, EntityUID id) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_find_child_by_id] No Scene bound to the ScriptSystem.");
		return 0;
	}

	std::optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[LUA] [entity_find_child_by_id] Entity of Id: {} not found.", self);
		return 0;
	}

	std::optional<Entity> child = entity->find_child_by_id(id);
	if (!child) {
		return 0;
	}

	return child->get_uid().value;
}

EntityUID entity_find_child_by_name(EntityUID self, const char* name) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		GL_LOG_ERROR("[LUA] [entity_find_child_by_name] No Scene bound to the ScriptSystem.");
		return 0;
	}

	std::optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[LUA] [entity_find_child_by_name] Entity of Id: {} not found.", self);
		return 0;
	}

	std::optional<Entity> child = entity->find_child_by_name(name);
	if (!child) {
		return 0;
	}

	return child->get_uid().value;
}

void transform_rotate(Transform* transform, float angle, glm::vec3 axis) {
	if (!transform) {
		GL_LOG_ERROR("[LUA] [transform_rotate]: Given transform instance is invalid.");
		return;
	}

	transform->rotate(angle, glm::normalize(axis));
}

glm::vec3 transform_get_forward(Transform* transform) {
	if (!transform) {
		GL_LOG_ERROR("[LUA] [transform_get_forward]: Given transform instance is invalid.");
		return VEC3_ZERO;
	}

	return transform->get_forward();
}

glm::vec3 transform_get_right(Transform* transform) {
	if (!transform) {
		GL_LOG_ERROR("[LUA] [transform_get_right]: Given transform instance is invalid.");
		return VEC3_ZERO;
	}

	return transform->get_right();
}

glm::vec3 transform_get_up(Transform* transform) {
	if (!transform) {
		GL_LOG_ERROR("[LUA] [transform_get_up]: Given transform instance is invalid.");
		return VEC3_ZERO;
	}

	return transform->get_up();
}

int input_get_key_down(int key_code) { return Input::is_key_pressed((KeyCode)key_code); }

int input_get_key_up(int key_code) { return Input::is_key_released((KeyCode)key_code); }

int input_get_mouse_down(int mouse_code) {
	return Input::is_mouse_pressed((MouseButton)mouse_code);
}

int input_get_mouse_up(int mouse_code) { return Input::is_mouse_released((MouseButton)mouse_code); }

void window_set_title(const char* title) {
	std::lock_guard<std::mutex> lock(g_script_mutex);

	Application* app = Application::get();
	if (!app) {
		GL_LOG_ERROR("[LUA] [window_set_title]: No Application instance found.");
		return;
	}

	app->get_window()->set_title(title);
}

int window_set_cursor_mode() {
	std::lock_guard<std::mutex> lock(g_script_mutex);

	Application* app = Application::get();
	if (!app) {
		GL_LOG_ERROR("[LUA] [Window_SetCursorMode]: No Application instance found.");
		return WindowCursorMode::WINDOW_CURSOR_MODE_DISABLED;
	}

	return app->get_window()->get_cursor_mode();
}

void Window_SetCursorMode(int mode) {
	std::lock_guard<std::mutex> lock(g_script_mutex);

	Application* app = Application::get();
	if (!app) {
		GL_LOG_ERROR("[LUA] [Window_SetCursorMode]: No Application instance found.");
		return;
	}

	app->get_window()->set_cursor_mode(static_cast<WindowCursorMode>(mode));
}

glm::vec2 window_set_size() {
	std::lock_guard<std::mutex> lock(g_script_mutex);

	Application* app = Application::get();
	if (!app) {
		GL_LOG_ERROR("[LUA] [window_set_size]: No Application instance found.");
		return { 0, 0 };
	}

	return app->get_window()->get_size();
}

} // extern "C"
} // namespace gl
