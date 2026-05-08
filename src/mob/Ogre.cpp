#include "mob/Ogre.h"

Ogre::Ogre(const glm::vec3& pos)
    : GameObject("Ogre")
    , ai(10.0f, 1.8f, 2.0f, 20, 2.0f, false, false)
{
    transform.position = pos;
    transform.scale = {1.5f, 1.5f, 1.5f};
    health.current = 100;
    health.max = 100;
    collider.halfExtents = {0.5f, 1.0f, 0.5f};
    collider.solid = true;
    renderable.tint = {0.4f, 0.3f, 0.5f};
}

void Ogre::update(float dt, const GameObject& player, const SpatialGrid& grid) {
    if (!health.alive()) {
        deathTimer += dt;
        if (deathTimer >= DEATH_DURATION)
            setDestroyed(true);
        return;
    }

    ai.update(dt, *this, player, grid);

    if (ai.wantsToAttack()) {
        if (!isWindingUp) {
            isWindingUp = true;
            attackWindUp = 0.7f;
        }
        attackWindUp -= dt;
        if (attackWindUp <= 0) {
            ai.resetCooldown();
            isWindingUp = false;
            ai.clearAttack();
        }
    }

    if (!isWindingUp) {
        glm::vec3 move = ai.moveDirection();
        transform.position += move * dt;
    }
    transform.position.y = 0;
}
