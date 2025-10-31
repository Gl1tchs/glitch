local Player = {}

function Player:on_create(entity)
    Engine.Log("Player script created!")

    local transform = Engine.GetTransform(entity)

    if transform ~= nil then
        transform.position.x = 100.0
        transform.position.y = 50.0
    end
end

function Player:on_update(entity, dt)
    local transform = Engine.GetTransform(entity)

    if transform ~= nil then
        transform.rotation.y = transform.rotation.y + (150.0 * dt)
    end
end

function Player:on_destroy(entity)
    print("Entity destroyed: " .. entity)
end

return Player
