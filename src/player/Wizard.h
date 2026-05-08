#pragma once
#include "entity/GameObject.h"
#include <glm/glm.hpp>

class Wizard : public GameObject {
public:
    Wizard();
    void update(float dt) override;

    void move(glm::vec3 dir, float dt);
    void aimAt(int screenX, int screenY, const class Camera& cam);
    void takeDamage(int dmg);
    bool isDead() const { return !health.alive(); }
    glm::vec3 aimDirection() const { return _aimDir; }

    int level = 1;
    int xp = 0;
    float mana = 100;
    float maxMana = 100;
    float speed = 5.0f;

    float globalCooldown = 0;

    void addXP(int amount);
    bool leveledUp() const { return _pendingLevelUp; }
    void clearLevelUp() { _pendingLevelUp = false; }

private:
    glm::vec3 _aimDir{0, 0, -1};
    bool _pendingLevelUp = false;
};
