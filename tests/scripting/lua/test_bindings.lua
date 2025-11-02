local Player = {}

local function assert_vec3_approx(v, x, y, z)
    local epsilon = 1e-4
    Debug.Assert(math.abs(v.x - x) < epsilon, "X component mismatch")
    Debug.Assert(math.abs(v.y - y) < epsilon, "Y component mismatch")
    Debug.Assert(math.abs(v.z - z) < epsilon, "Z component mismatch")
    Debug.Log("Asserted vector: (%f, %f, %f) vs expected (%f, %f, %f)", v.x, v.y, v.z, x, y, z)
end

function Player:on_create(entity)
    Debug.Log("--- Starting LUA Entity & Transform Tests for entity %d ---", entity)

    -- Existing Entity Tests
    Debug.Assert(Entity.IsValid(entity), "Initial entity must be valid")
    Debug.Assert(Entity.FindById(entity) == entity, "FindEntityById must return self")

    local new_entity = Entity.Create("Bob")
    Debug.Assert(Entity.IsValid(new_entity), "New entity must be valid")
    Debug.Assert(Entity.FindByName("Bob") == new_entity, "FindEntityByName must work")

    Entity.SetName(new_entity, "John")
    Debug.Assert(Entity.GetName(new_entity) == "John", "EntityGetName/SetName must work")

    Debug.Assert(Entity.GetParent(new_entity) == 0, "Initial parent must be 0")
    Entity.SetParent(new_entity, entity)
    Debug.Assert(Entity.GetParent(new_entity) == entity, "EntitySetParent must work")

    Debug.Assert(Entity.FindChildById(entity, new_entity) == new_entity, "FindChildById must work")
    Debug.Assert(Entity.FindChildByName(entity, "John") == new_entity, "FindChildByName must work")

    Entity.Destroy(new_entity)
    Debug.Assert(not Entity.IsValid(new_entity), "EntityDestroy must invalidate entity")

    -- Transform Tests using new metatable methods
    local transform = Entity.GetTransform(entity)
    Debug.Assert(transform ~= nil, "GetTransform must return a valid transform")

    -- Test initial position (already checked by user, keeping it)
    Debug.Assert(transform.position.x == 0.0, "Initial position X must be 0.0")

    -- Test initial orientation (assuming identity/forward=(0,0,-1), right=(1,0,0), up=(0,1,0))
    Debug.Log("Testing initial orientation...")
    assert_vec3_approx(transform:GetForward(), 0.0, 0.0, -1.0)
    assert_vec3_approx(transform:GetRight(), 1.0, 0.0, 0.0)
    assert_vec3_approx(transform:GetUp(), 0.0, 1.0, 0.0)

    Debug.Log("Testing 90 degree rotation around Up axis...")

    local up_axis = Vec3(0.0, 1.0, 0.0)
    transform:Rotate(90.0, up_axis)

    -- Check new orientation: 
    -- Old Forward (0,0,-1) rotates to New Forward (-1,0,0) (standard rotation)
    -- Old Right (1,0,0) rotates to New Right (0,0,-1)

    local new_fwd = transform:GetForward()
    local new_right = transform:GetRight()
    local new_up = transform:GetUp()

    assert_vec3_approx(new_fwd, -1.0, 0.0, 0.0)
    assert_vec3_approx(new_right, 0.0, 0.0, -1.0)

    -- Up axis should be unchanged by rotation around itself
    assert_vec3_approx(new_up, 0.0, 1.0, 0.0)

    Debug.Log("--- All LUA Tests Passed! ---")

end

function Player:on_update(entity, dt)
end

function Player:on_destroy(entity)
end

return Player
