ffi = require('ffi')

ffi.cdef [[
    /* ---------------- Math Types ---------------- */
    typedef struct { float x, y; } Vec2;
    typedef struct { float x, y, z; } Vec3;
    typedef struct { float x, y, z, w; } Vec4;

    /* ---------------- Core Types ---------------- */

    typedef struct Transform {
        struct Transform* parent;
        Vec3 position;
        Vec3 rotation;
        Vec3 scale;
    } Transform;

    /* ---------------- Global Engine Functions ---------------- */
    void Log(const char* message);
    void Assert(bool condition, const char* message);

    uint32_t FindEntityById(uint32_t p_id);
    uint32_t FindEntityByName(const char* p_name);

    Transform* GetTransform(uint32_t self);

    int GetKeyDown(int key_code);
    int GetKeyUp(int key_code);
    int GetMouseDown(int mouse_code);
    int GetMouseUp(int mouse_code);

    /* ---------------- Method Implementations ---------------- */

    /* Entity Methods */
    uint32_t Entity_Create(const char* name);
    void Entity_Destroy(uint32_t self);

    int Entity_IsValid(uint32_t self);

    const char* Entity_GetName(uint32_t self);
    void Entity_SetName(uint32_t self, const char* name);

    uint32_t Entity_GetParent(uint32_t self);
    void Entity_SetParent(uint32_t self, uint32_t parent);

    uint32_t Entity_FindChildById(uint32_t self, uint32_t id);
    uint32_t Entity_FindChildByName(uint32_t self, const char* name);

    /* Transform Methods */
    void Transform_Rotate(Transform* transform, float angle, Vec3 axis);
    Vec3 Transform_GetForward(Transform* transform);
    Vec3 Transform_GetRight(Transform* transform);
    Vec3 Transform_GetUp(Transform* transform);

    /* Window Methods */
    void Window_SetTitle(const char* title);
    int Window_GetCursorMode();
    void Window_SetCursorMode(int mode);
    Vec2 Window_GetSize();

    /* C System Functions */
    void free(void* ptr);
]]

Engine = {}
Debug = {}
Entity = {}
Input = {}
Window = {}

local C = {}
Engine.C_Functions = C

-- Type wrappers

Vec2 = function(x, y)
    -- Creates a new Vec2 cdata object, fills with 0 if args are nil
    return ffi.new("Vec2", x or 0, y or 0)
end

Vec3 = function(x, y, z)
    -- Creates a new Vec3 cdata object, fills with 0 if args are nil
    return ffi.new("Vec3", x or 0, y or 0, z or 0)
end

Vec4 = function(x, y, z, w)
    -- Creates a new Vec4 cdata object, fills with 0 if args are nil
    return ffi.new("Vec4", x or 0, y or 0, z or 0, w or 0)
end

-- Debug Utilities --

Debug.Log = function(...)
    C.Log(string.format(...))
end
Debug.Assert = function(condition, ...)
    C.Assert(condition, string.format(...))
end

-- Entity Methods --

Entity.FindById = function(id)
    return C.FindEntityById(id)
end
Entity.FindByName = function(name)
    return C.FindEntityByName(name)
end
Entity.GetTransform = function(entity_id)
    return C.GetTransform(entity_id)
end
Entity.Create = function(name)
    return C.Entity_Create(name)
end
Entity.Destroy = function(entity)
    return C.Entity_Destroy(entity)
end
Entity.IsValid = function(entity)
    return C.Entity_IsValid(entity) ~= 0
end
Entity.GetName = function(entity)
    local c_ptr = C.Entity_GetName(entity)

    if c_ptr == nil then
        return ""
    end

    local name = ffi.string(c_ptr)

    -- Tell the FFI garbage collector to call C.free(c_ptr)
    -- when the 'c_ptr' object is no longer reachable.
    ffi.gc(c_ptr, C.free)

    return name
end
Entity.SetName = function(entity, name)
    return C.Entity_SetName(entity, name)
end
Entity.GetParent = function(entity)
    return C.Entity_GetParent(entity)
end
Entity.SetParent = function(entity, parent)
    return C.Entity_SetParent(entity, parent)
end
Entity.FindChildById = function(entity, id)
    return C.Entity_FindChildById(entity, id)
end
Entity.FindChildByName = function(entity, name)
    return C.Entity_FindChildByName(entity, name)
end

-- Transform methods

Transform = function()
    -- Creates a new, zero-initialized Transform cdata object
    -- (parent will be nil, vectors will be 0,0,0)
    return ffi.new("Transform")
end

ffi.metatype("Transform", {
    __index = {
        Rotate = function(self, angle, axis)
            C.Transform_Rotate(self, angle, axis)
        end,
        GetForward = function(self)
            return C.Transform_GetForward(self)
        end,
        GetRight = function(self)
            return C.Transform_GetRight(self)
        end,
        GetUp = function(self)
            return C.Transform_GetUp(self)
        end
    }
})

-- Input API --

Input.GetKeyDown = function(key)
    return C.GetKeyDown(key) ~= 0
end
Input.GetKeyUp = function(key)
    return C.GetKeyUp(key) ~= 0
end
Input.GetMouseDown = function(btn)
    return C.GetMouseDown(btn) ~= 0
end
Input.GetMouseUp = function(btn)
    return C.GetMouseUp(btn) ~= 0
end

-- Window Methods --

Window.SetTitle = function(title)
    return C.Window_SetTitle(title);
end
Window.GetCursorMode = function()
    return C.Window_GetCursorMode();
end
Window.SetCursorMode = function(mode)
    C.Window_SetCursorMode(mode);
end
Window.GetSize = function()
    return C.Window_GetSize()
end;

local CursorMode = {
    Normal = 0,
    Hidden = 1,
    Disabled = 2,
    Captured = 3
}

Window.CursorMode = CursorMode

-- ---- KeyCode and MouseCode enums ----

local KeyCode = {
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

Input.KeyCode = KeyCode

local MouseCode = {
    Left = 0,
    Right = 1,
    Middle = 2
}

Input.MouseCode = MouseCode
