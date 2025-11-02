local RotatingCubes = {}
RotatingCubes.speed = 0.0
RotatingCubes.my_boolvar = false
RotatingCubes.stringvar = "Hello LUA!"

function RotatingCubes:on_create(entity)
    Engine.Log(self.stringvar)
    Engine.Log("Player created with ID: %d", entity)

    local position = Engine.Vec3(0.0, 5.0, 0.0)
    position.x = 5.0

    local transform = Engine.GetTransform(entity)
    if transform ~= nil then
        transform:SetPosition(position)
    end
end

function RotatingCubes:on_update(entity, dt)
    local transform = Engine.GetTransform(entity)
    if transform == nil then
        return
    end

    if Engine.GetKeyDown(Engine.KeyCode.E) then
        self.speed = self.speed + 1.0
    end

    if Engine.GetKeyDown(Engine.KeyCode.Q) then
        self.speed = self.speed - 1.0
    end

    transform.rotation.y = transform.rotation.y + (self.speed * dt)
end

function RotatingCubes:on_destroy(entity)
end

return RotatingCubes
