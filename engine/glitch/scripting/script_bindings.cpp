#include "glitch/core/application.h"
#include "glitch/core/event/input.h"
#include "glitch/core/transform.h"
#include "glitch/scene/scene.h"
#include "glitch/scripting/script_system.h"

#include <lua.hpp>

#include "ffi_lua_resource.gen.h"

namespace gl {

std::mutex g_script_mutex;

typedef uint32_t EntityUID;

extern "C" {

// --- Global functions ---

void Log(const char* message);
void Assert(int condition, const char* message);

EntityUID FindEntityById(EntityUID p_id);
EntityUID FindEntityByName(const char* p_name);

// --- Entity Methods  ---

Transform* GetTransform(EntityUID self);

EntityUID Entity_Create(const char* name);
void Entity_Destroy(EntityUID self);

int Entity_IsValid(EntityUID self);

const char* Entity_GetName(EntityUID self);
void Entity_SetName(EntityUID self, const char* name);

EntityUID Entity_GetParent(EntityUID self);
void Entity_SetParent(EntityUID self, EntityUID parent);

EntityUID Entity_FindChildById(EntityUID self, EntityUID id);
EntityUID Entity_FindChildByName(EntityUID self, const char* name);

// --- Transform Methods ---

void Transform_Rotate(Transform* transform, float angle, glm::vec3 axis);
glm::vec3 Transform_GetForward(Transform* transform);
glm::vec3 Transform_GetRight(Transform* transform);
glm::vec3 Transform_GetUp(Transform* transform);

// --- Input ---

int GetKeyDown(int key_code);
int GetKeyUp(int key_code);
int GetMouseDown(int mouse_code);
int GetMouseUp(int mouse_code);

// --- Window Methods ---

void Window_SetTitle(const char* title);
int Window_GetCursorMode();
void Window_SetCursorMode(int mode);
glm::vec2 Window_GetSize();

} // extern "C"

static void _run_string(lua_State* L, const char* p_code) {
	if (luaL_dostring(L, p_code) != LUA_OK) {
		GL_LOG_ERROR("[LUA] FFI Bind Error: {}", lua_tostring(L, -1));
		lua_pop(L, 1);
		GL_ASSERT(false); // FFI scripts must compile
	}
}

static void _run_string(lua_State* L, const std::string& p_code) {
	_run_string(L, p_code.c_str());
}

static void _bind_function(lua_State* L, const char* p_func_path,
		const char* p_func_def, uintptr_t p_fnptr) {
	_run_string(L,
			std::format("{} = ffi.cast('{}', {}ULL)", p_func_path, p_func_def,
					p_fnptr));
}

void _register_bindings(lua_State* L) {
	// Load and run the FFI script
	_run_string(L, lua_bindings::ffi_source);

	/* ---------------- Utility Functions ---------------- */
	_bind_function(L, "Engine.C_Functions.Log", "void (*)(const char*)",
			(uintptr_t)&Log);
	_bind_function(L, "Engine.C_Functions.Assert", "void (*)(int, const char*)",
			(uintptr_t)&Assert);

	/* ---------------- Global Engine Functions ---------------- */
	_bind_function(L, "Engine.C_Functions.FindEntityById",
			"uint32_t (*)(uint32_t)", (uintptr_t)&FindEntityById);
	_bind_function(L, "Engine.C_Functions.FindEntityByName",
			"uint32_t (*)(const char*)", (uintptr_t)&FindEntityByName);

	_bind_function(L, "Engine.C_Functions.GetTransform",
			"Transform* (*)(uint32_t)", (uintptr_t)&GetTransform);

	/* ---------------- Entity Methods ---------------- */
	_bind_function(L, "Engine.C_Functions.Entity_Create",
			"uint32_t (*)(const char*)", (uintptr_t)&Entity_Create);
	_bind_function(L, "Engine.C_Functions.Entity_Destroy", "void (*)(uint32_t)",
			(uintptr_t)&Entity_Destroy);
	_bind_function(L, "Engine.C_Functions.Entity_IsValid", "int (*)(uint32_t)",
			(uintptr_t)&Entity_IsValid);
	_bind_function(L, "Engine.C_Functions.Entity_GetName",
			"const char* (*)(uint32_t)", (uintptr_t)&Entity_GetName);
	_bind_function(L, "Engine.C_Functions.Entity_SetName",
			"void (*)(uint32_t, const char*)", (uintptr_t)&Entity_SetName);
	_bind_function(L, "Engine.C_Functions.Entity_GetParent",
			"uint32_t (*)(uint32_t)", (uintptr_t)&Entity_GetParent);
	_bind_function(L, "Engine.C_Functions.Entity_SetParent",
			"void (*)(uint32_t, uint32_t)", (uintptr_t)&Entity_SetParent);
	_bind_function(L, "Engine.C_Functions.Entity_FindChildById",
			"uint32_t (*)(uint32_t, uint32_t)",
			(uintptr_t)&Entity_FindChildById);
	_bind_function(L, "Engine.C_Functions.Entity_FindChildByName",
			"uint32_t (*)(uint32_t, const char*)",
			(uintptr_t)&Entity_FindChildByName);

	/* ---------------- Transform Methods ---------------- */
	_bind_function(L, "Engine.C_Functions.Transform_Rotate",
			"void (*)(Transform*, float, Vec3)", (uintptr_t)&Transform_Rotate);
	_bind_function(L, "Engine.C_Functions.Transform_GetForward",
			"Vec3 (*)(Transform*)", (uintptr_t)&Transform_GetForward);
	_bind_function(L, "Engine.C_Functions.Transform_GetRight",
			"Vec3 (*)(Transform*)", (uintptr_t)&Transform_GetRight);
	_bind_function(L, "Engine.C_Functions.Transform_GetUp",
			"Vec3 (*)(Transform*)", (uintptr_t)&Transform_GetUp);

	/* ---------------- Input API ---------------- */
	_bind_function(L, "Engine.C_Functions.GetKeyDown", "int (*)(int)",
			(uintptr_t)&GetKeyDown);
	_bind_function(L, "Engine.C_Functions.GetKeyUp", "int (*)(int)",
			(uintptr_t)&GetKeyUp);
	_bind_function(L, "Engine.C_Functions.GetMouseDown", "int (*)(int)",
			(uintptr_t)&GetMouseDown);
	_bind_function(L, "Engine.C_Functions.GetMouseUp", "int (*)(int)",
			(uintptr_t)&GetMouseUp);

	/* ---------------- Window Methods ---------------- */
	_bind_function(L, "Engine.C_Functions.Window_SetTitle",
			"void (*)(const char*)", (uintptr_t)&Window_SetTitle);
	_bind_function(L, "Engine.C_Functions.Window_GetCursorMode", "int (*)()",
			(uintptr_t)&Window_GetCursorMode);
	_bind_function(L, "Engine.C_Functions.Window_SetCursorMode",
			"void (*)(int)", (uintptr_t)&Window_SetCursorMode);
	_bind_function(L, "Engine.C_Functions.Window_GetSize", "Vec2 (*)()",
			(uintptr_t)&Window_GetSize);
}

extern "C" {

void Log(const char* message) { GL_LOG_INFO("[LUA] {}", message); }

void Assert(int condition, const char* message) {
	GL_ASSERT(condition, message);
}

EntityUID FindEntityById(EntityUID p_id) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return 0;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(p_id);
	return entity ? p_id : 0;
}

EntityUID FindEntityByName(const char* p_name) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return 0;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_name(p_name);
	return entity ? (*entity).get_uid().value : 0;
}

Transform* GetTransform(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running() || self == 0) {
		return nullptr;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (entity) {
		return &(*entity).get_transform();
	}
	return nullptr;
}

EntityUID Entity_Create(const char* name) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return 0;
	}

	Entity entity = ScriptSystem::get_scene()->create(name);
	return entity.get_uid().value;
}

void Entity_Destroy(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return;
	}

	Scene* scene = ScriptSystem::get_scene();
	Optional<Entity> entity = scene->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[SCRIPT] Unable to destroy entity.");
		return;
	}

	scene->destroy(*entity);
}

int Entity_IsValid(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return false;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	return entity && entity->is_valid();
}

const char* Entity_GetName(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return "";
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		return "";
	}

	return entity->get_name().c_str();
}

void Entity_SetName(EntityUID self, const char* name) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		return;
	}

	entity->set_name(name);
}

EntityUID Entity_GetParent(EntityUID self) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return 0;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		return 0;
	}

	if (Optional<Entity> parent = entity->get_parent()) {
		return parent->get_uid().value;
	}

	return 0;
}

void Entity_SetParent(EntityUID self, EntityUID parent_id) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[SCRIPT] Entity of Id: {} not found.", self);
		return;
	}

	Optional<Entity> parent = ScriptSystem::get_scene()->find_by_id(parent_id);
	if (!parent) {
		GL_LOG_ERROR("[SCRIPT] Parent of Id: {} not found.", self);
		return;
	}

	entity->set_parent(*parent);
}

EntityUID Entity_FindChildById(EntityUID self, EntityUID id) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return 0;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[SCRIPT] Entity of Id: {} not found.", self);
		return 0;
	}

	Optional<Entity> child = entity->find_child_by_id(id);
	if (!child) {
		return 0;
	}

	return child->get_uid().value;
}

EntityUID Entity_FindChildByName(EntityUID self, const char* name) {
	std::lock_guard<std::mutex> lock(g_script_mutex);
	if (!ScriptSystem::is_running()) {
		return 0;
	}

	Optional<Entity> entity = ScriptSystem::get_scene()->find_by_id(self);
	if (!entity) {
		GL_LOG_ERROR("[SCRIPT] Entity of Id: {} not found.", self);
		return 0;
	}

	Optional<Entity> child = entity->find_child_by_name(name);
	if (!child) {
		return 0;
	}

	return child->get_uid().value;
}

void Transform_Rotate(Transform* transform, float angle, glm::vec3 axis) {
	if (!transform) {
		// TODO: proper error handling
		return;
	}

	transform->rotate(angle, glm::normalize(axis));
}

glm::vec3 Transform_GetForward(Transform* transform) {
	if (!transform) {
		return VEC3_ZERO;
	}

	return transform->get_forward();
}

glm::vec3 Transform_GetRight(Transform* transform) {
	if (!transform) {
		return VEC3_ZERO;
	}

	return transform->get_right();
}

glm::vec3 Transform_GetUp(Transform* transform) {
	if (!transform) {
		return VEC3_ZERO;
	}

	return transform->get_up();
}

int GetKeyDown(int key_code) {
	return Input::is_key_pressed((KeyCode)key_code);
}

int GetKeyUp(int key_code) { return Input::is_key_released((KeyCode)key_code); }

int GetMouseDown(int mouse_code) {
	return Input::is_mouse_pressed((MouseButton)mouse_code);
}

int GetMouseUp(int mouse_code) {
	return Input::is_mouse_released((MouseButton)mouse_code);
}

void Window_SetTitle(const char* title) {
	std::lock_guard<std::mutex> lock(g_script_mutex);

	Application* app = Application::get_instance();
	if (!app) {
		// TODO: proper error handling
		return;
	}

	app->get_window()->set_title(title);
}

int Window_GetCursorMode() {
	std::lock_guard<std::mutex> lock(g_script_mutex);

	Application* app = Application::get_instance();
	if (!app) {
		// TODO: proper error handling
		return WindowCursorMode::WINDOW_CURSOR_MODE_DISABLED;
	}

	return app->get_window()->get_cursor_mode();
}

void Window_SetCursorMode(int mode) {
	std::lock_guard<std::mutex> lock(g_script_mutex);

	Application* app = Application::get_instance();
	if (!app) {
		// TODO: proper error handling
		return;
	}

	app->get_window()->set_cursor_mode(static_cast<WindowCursorMode>(mode));
}

glm::vec2 Window_GetSize() {
	std::lock_guard<std::mutex> lock(g_script_mutex);

	Application* app = Application::get_instance();
	if (!app) {
		// TODO: proper error handling
		return { 0, 0 };
	}

	return app->get_window()->get_size();
}

} // extern "C"
} // namespace gl
