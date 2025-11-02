local MyEntity = {}
MyEntity.title = "Window Title"

function MyEntity:on_create(entity)
    Window.SetTitle(self.title)

    Window.SetCursorMode(Window.CursorMode.Disabled)
end

function MyEntity:on_update(entity, dt)
    if (Input.GetKeyDown(Input.KeyCode.Escape)) then
        Window.SetCursorMode(Window.CursorMode.Normal)
    end

end

function MyEntity:on_destroy(entity)
    Window.SetCursorMode(Window.CursorMode.Normal)
end

return MyEntity
