local RotatingCubes = {}
RotatingCubes.speed = 0.0
RotatingCubes.my_boolvar = false
RotatingCubes.stringvar = "Hello LUA!"

function RotatingCubes:on_create(entity)
    print(self.stringvar)
end

function RotatingCubes:on_update(entity, dt)
    local transform = Engine.GetTransform(entity)

    if Engine.GetKeyDown(Engine.KeyCode.W) then
        self.speed = self.speed + 1.0
    end

    if Engine.GetKeyDown(Engine.KeyCode.S) then
        self.speed = self.speed - 1.0
    end

    if transform ~= nil then
        transform.rotation.y = transform.rotation.y + (self.speed * dt)
    end
end

function RotatingCubes:on_destroy(entity)
end

return RotatingCubes
