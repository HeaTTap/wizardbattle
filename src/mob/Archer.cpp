#include "mob/Archer.h"

Archer::Archer(const glm::vec3& pos)
    : GameObject("Archer")
    , ai(15.0f, 10.0f, 2.0f, 12, 3.0f, true, false)
{
    transform.position = pos;
    health.current = 30;
    health.max = 30;
    collider.halfExtents = {0.3f, 0.7f, 0.3f};
    collider.solid = true;
    renderable.tint = {0.6f, 0.3f, 0.1f};
}

void Archer::update(float dt, const GameObject& player, const SpatialGrid& grid) {
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
