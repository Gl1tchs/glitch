#include <lua.hpp>

#include "glitch/core/event/input.h"
#include "glitch/core/transform.h"
#include "glitch/scene/scene.h"
#include "glitch/scripting/script_system.h"

namespace gl {

std::mutex g_script_mutex;

extern "C" {

// Lua: Engine.Log("Hello")
void Log(const char* message) { GL_LOG_INFO("[LUA] {}", message); }

// Lua: local transform = Engine.GetTransform(entity_id)
Transform* GetTransform(uint64_t p_uid) {
	std::lock_guard<std::mutex> lock(g_script_mutex);

	Entity entity = ScriptSystem::get_scene()->find_by_id(p_uid);
	if (entity) {
		return &entity.get_transform();
	}
	return nullptr;
}

bool GetKeyDown(KeyCode key_code) { return Input::is_key_pressed(key_code); }

bool GetKeyUp(KeyCode key_code) { return Input::is_key_released(key_code); }

bool GetMouseDown(MouseButton mouse_code) {
	return Input::is_mouse_pressed(mouse_code);
}

bool GetMouseUp(MouseButton mouse_code) {
	return Input::is_mouse_released(mouse_code);
}
}

static void _run_string(lua_State* L, const std::string& p_code) {
	if (luaL_dostring(L, p_code.c_str()) != LUA_OK) {
		GL_LOG_ERROR("[LUA] FFI Bind Error: {}", lua_tostring(L, -1));
		lua_pop(L, 1);
		GL_ASSERT(false); // they must work
	}
}

void _register_bindings(lua_State* L) {
	_run_string(L, "ffi = require('ffi')");

	_run_string(L, R"lua(
        ffi.cdef[[
            /* ---------------- Math Types ---------------- */
        
            typedef struct { float x, y, z; } Vec3;
            
            /* ---------------- Utility Functions ---------------- */

            void Log(const char* message);

            /* ---------------- Components ---------------- */

            typedef struct Transform {
                struct Transform* parent; 
                Vec3 position;
                Vec3 rotation;
                Vec3 scale;
            } Transform;

            Transform* GetTransform(uint32_t entity_id);

            /* ---------------- Input API ---------------- */ 
            
            bool GetKeyDown(int key_code);
            bool GetKeyUp(int key_code);
            
            bool GetMouseDown(int mouse_code);
            bool GetMouseUp(int mouse_code);
        ]]
    )lua");

	// New empty table for namespacing
	_run_string(L, "Engine = {}");

	// Bind Engine.Log
	uintptr_t log_ptr = (uintptr_t)&Log;
	_run_string(L,
			"Engine.Log = ffi.cast('void (*)(const char*)', " +
					std::to_string(log_ptr) + "ULL)");

	// Bind Engine.GetTransform
	uintptr_t get_transform_ptr = (uintptr_t)&GetTransform;
	_run_string(L,
			"Engine.GetTransform = ffi.cast('Transform* "
			"(*)(uint32_t)', " +
					std::to_string(get_transform_ptr) + "ULL)");

	// Bind Engine.GetKeyDown
	uintptr_t get_key_down_ptr = (uintptr_t)&GetKeyDown;
	_run_string(L,
			"Engine.GetKeyDown = ffi.cast('bool (*)(int)', " +
					std::to_string(get_key_down_ptr) + "ULL)");

	// Bind Engine.GetKeyUp
	uintptr_t get_key_up_ptr = (uintptr_t)&GetKeyUp;
	_run_string(L,
			"Engine.GetKeyUp = ffi.cast('bool (*)(int)', " +
					std::to_string(get_key_up_ptr) + "ULL)");

	// Bind Engine.GetMouseDown
	uintptr_t get_mouse_down_ptr = (uintptr_t)&GetMouseDown;
	_run_string(L,
			"Engine.GetMouseDown = ffi.cast('bool (*)(int)', " +
					std::to_string(get_mouse_down_ptr) + "ULL)");

	// Bind Engine.GetMouseUp
	uintptr_t get_mouse_up_ptr = (uintptr_t)&GetMouseUp;
	_run_string(L,
			"Engine.GetMouseUp = ffi.cast('bool (*)(int)', " +
					std::to_string(get_mouse_up_ptr) + "ULL)");

	// Create lua table for KeyCode
	_run_string(L, R"lua(
        KeyCode = {
            Space = 32,
            Apostrophe = 39,
            Comma = 44,
            Minus = 45,
            Period = 46,
            Slash = 47,
            KeyPad0 = 48,
            KeyPad1 = 49,
            KeyPad2 = 50,
            KeyPad3 = 51,
            KeyPad4 = 52,
            KeyPad5 = 53,
            KeyPad6 = 54,
            KeyPad7 = 55,
            KeyPad8 = 56,
            KeyPad9 = 57,
            Semicolon = 59,
            Equal = 61,
            A = 65,
            B = 66,
            C = 67,
            D = 68,
            E = 69,
            F = 70,
            G = 71,
            H = 72,
            I = 73,
            J = 74,
            K = 75,
            L = 76,
            M = 77,
            N = 78,
            O = 79,
            P = 80,
            Q = 81,
            R = 82,
            S = 83,
            T = 84,
            U = 85,
            V = 86,
            W = 87,
            X = 88,
            Y = 89,
            Z = 90,
            LeftBracket = 91,
            Backslash = 92,
            RightBracket = 93,
            GraveAccent = 96,
            World1 = 161,
            World2 = 162,
            Escape = 256,
            Enter = 257,
            Tab = 258,
            Backspace = 259,
            Insert = 260,
            Delete = 261,
            Right = 262,
            Left = 263,
            Down = 264,
            Up = 265,
            PageUp = 266,
            PageDown = 267,
            Home = 268,
            End = 269,
            CapsLock = 280,
            ScrollLock = 281,
            NumLock = 282,
            PrintScreen = 283,
            Pause = 284,
            F1 = 290,
            F2 = 291,
            F3 = 292,
            F4 = 293,
            F5 = 294,
            F6 = 295,
            F7 = 296,
            F8 = 297,
            F9 = 298,
            F10 = 299,
            F11 = 300,
            F12 = 301,
            F13 = 302,
            F14 = 303,
            F15 = 304,
            F16 = 305,
            F17 = 306,
            F18 = 307,
            F19 = 308,
            F20 = 309,
            F21 = 310,
            F22 = 311,
            F23 = 312,
            F24 = 313,
            F25 = 314,
            Kp0 = 320,
            Kp1 = 321,
            Kp2 = 322,
            Kp3 = 323,
            Kp4 = 324,
            Kp5 = 325,
            Kp6 = 326,
            Kp7 = 327,
            Kp8 = 328,
            Kp9 = 329,
            KpDecimal = 330,
            KpDivide = 331,
            KpMultiply = 332,
            KpSubtract = 333,
            KpAdd = 334,
            KpEnter = 335,
            KpEqual = 336,
            LeftShift = 340,
            LeftControl = 341,
            LeftAlt = 342,
            LeftSuper = 343,
            RightShift = 344,
            RightControl = 345,
            RightAlt = 346,
            RightSuper = 347,
            Menu = 348
        }
        
        Engine.KeyCode = KeyCode
    )lua");

	// Create MouseCode table
	_run_string(L, R"lua(
        MouseCode = {
            Left = 0,
            Right = 1,
            Middle = 2
        }

        Engine.MouseCode = MouseCode
    )lua");
}

} //namespace gl