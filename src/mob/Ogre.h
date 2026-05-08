#pragma once
#include "entity/GameObject.h"
#include "mob/MobAI.h"

class Ogre : public GameObject {
public:
    Ogre(const glm::vec3& pos);
    void update(float dt, const GameObject& player, const SpatialGrid& grid);

    MobAI ai;
    int xpValue() const { return 70; }

    float attackWindUp = 0;
    bool isWindingUp = false;

    float deathTimer = 0;
    static constexpr float DEATH_DURATION = 3.0f;
};
