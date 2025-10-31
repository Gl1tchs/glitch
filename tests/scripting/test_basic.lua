local Player = {}

function Player:on_create(entity)
    -- 'Engine' exists globally, and 'Log' is a direct C++ call
    Engine.Log("Player script created!")

    -- Get the *direct C++ pointer* to the component
    local transform = Engine.GetTransform(entity)

    if transform ~= nil then
        -- This is not a copy! You are modifying
        -- C++ memory directly from Lua.
        -- This is why FFI is so fast.
        transform.position.x = 100.0
        transform.position.y = 50.0
    end
end

function Player:on_update(entity, dt)
    local transform = Engine.GetTransform(entity)

    if transform ~= nil then
        transform.rotation.z = transform.rotation.z + (1.5 * dt)
    end
end

function Player:on_destroy(entity)
    print("Entity destroyed: " .. entity)
end

return Player
