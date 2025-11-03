local RotatingCubes = {}
RotatingCubes.speed = 0.0
RotatingCubes.my_boolvar = false
RotatingCubes.stringvar = "Hello LUA!"

function RotatingCubes:on_create(entity)
    Debug.Log(self.stringvar)
    Debug.Log("Player created with ID: %d", entity)

    local position = Vec3(0.0, 5.0, 0.0)
    position.x = 5.0
end

function RotatingCubes:on_update(entity, dt)
    local transform = Entity.GetTransform(entity)
    if transform == nil then
        Debug.Assert(false)
        return
    end

    if Input.GetKeyDown(Input.KeyCode.E) then
        self.speed = self.speed + 1.0
    end

    if Input.GetKeyDown(Input.KeyCode.Q) then
        self.speed = self.speed - 1.0
    end

    transform.rotation.y = transform.rotation.y + (self.speed * dt)
end

function RotatingCubes:on_destroy(entity)
end

return RotatingCubes
