local RotatingCubes = {}

local speed = 0.0

function RotatingCubes:on_create(entity)
end

function RotatingCubes:on_update(entity, dt)
    local transform = Engine.GetTransform(entity)

    if Engine.GetKeyDown(Engine.KeyCode.W) then
        speed = speed + 1.0
    end

    if Engine.GetKeyDown(Engine.KeyCode.S) then
        speed = speed - 1.0
    end

    if transform ~= nil then
        transform.rotation.y = transform.rotation.y + (speed * dt)
    end
end

function RotatingCubes:on_destroy(entity)
end

return RotatingCubes
