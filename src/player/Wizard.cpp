#include "player/Wizard.h"
#include "core/Camera.h"

Wizard::Wizard() : GameObject("Player") {
    transform.position = {100, 0, 180};
    health.current = 100;
    health.max = 100;
    collider.halfExtents = {0.4f, 0.8f, 0.4f};
    collider.solid = false;
}

void Wizard::update(float dt) {
    mana = glm::min(mana + 5.0f * dt, maxMana);
    if (globalCooldown > 0) globalCooldown -= dt;
}

void Wizard::move(glm::vec3 dir, float dt) {
    if (glm::length(dir) > 1.0f) dir = glm::normalize(dir);
    transform.position += dir * speed * dt;

    transform.position.x = glm::clamp(transform.position.x, 1.0f, 199.0f);
    transform.position.z = glm::clamp(transform.position.z, 1.0f, 199.0f);
}

void Wizard::aimAt(int screenX, int screenY, const Camera& cam) {
    float ndcX = (screenX / (float)cam.screenWidth()) * 2.0f - 1.0f;
    float ndcY = 1.0f - (screenY / (float)cam.screenHeight()) * 2.0f;
    glm::mat4 invVP = glm::inverse(cam.proj() * cam.view());
    glm::vec4 near = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    near /= near.w;
    glm::vec4 far = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
    far /= far.w;

    glm::vec3 dir = glm::normalize(glm::vec3(far - near));
    if (glm::abs(dir.y) > 0.001f) {
        float t = -near.y / dir.y;
        if (t > 0) {
            glm::vec3 hit = glm::vec3(near) + dir * t;
            _aimDir = glm::normalize(hit - transform.position);
        }
    }
}

void Wizard::takeDamage(int dmg) {
    health.takeDamage(dmg);
}

void Wizard::addXP(int amount) {
    xp += amount;
    int xpNeeded = (int)(100.0f * powf(1.5f, level - 1));
    while (xp >= xpNeeded && level < 10) {
        xp -= xpNeeded;
        level++;
        _pendingLevelUp = true;
        maxMana = 100.0f + 15.0f * (level - 1);
        mana = maxMana;
        health.max = 100 + 10 * (level - 1);
        health.current = health.max;
        xpNeeded = (int)(100.0f * powf(1.5f, level - 1));
    }
}
