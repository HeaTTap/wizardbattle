#include "mob/Goblin.h"

Goblin::Goblin(const glm::vec3& pos)
    : GameObject("Goblin")
    , ai(12.0f, 1.5f, 1.0f, 8, 4.0f, false, true)
{
    transform.position = pos;
    health.current = 40;
    health.max = 40;
    collider.halfExtents = {0.3f, 0.6f, 0.3f};
    collider.solid = true;
    renderable.tint = {0.2f, 0.7f, 0.2f};
}

void Goblin::update(float dt, const GameObject& player, const SpatialGrid& grid) {
    if (!health.alive()) {
        deathTimer += dt;
        if (deathTimer >= DEATH_DURATION)
            setDestroyed(true);
        return;
    }

    ai.update(dt, *this, player, grid);
    glm::vec3 move = ai.moveDirection();
    transform.position += move * dt;
    transform.position.y = 0;
}
