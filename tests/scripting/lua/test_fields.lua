local Player = {}
Player.name = "Player"
Player.health = 0.0
Player.alive = false

function Player:on_create(entity)
    self.health = 100.0
    self.alive = true
end

function Player:on_update(entity, dt)
    self.health = self.health - 20.0
    self.name = "Player1"
end

function Player:on_destroy(entity)
    self.health = 0.0
    self.alive = false
end

return Player
