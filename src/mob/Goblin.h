#pragma once
#include "entity/GameObject.h"
#include "mob/MobAI.h"

class Goblin : public GameObject {
public:
    Goblin(const glm::vec3& pos);
    void update(float dt, const GameObject& player, const SpatialGrid& grid);

    MobAI ai;
    int xpValue() const { return 30; }

    float deathTimer = 0;
    static constexpr float DEATH_DURATION = 3.0f;
};
