#pragma once
#include "entity/GameObject.h"
#include "mob/Goblin.h"
#include "mob/Archer.h"
#include "mob/Ogre.h"
#include <vector>
#include <memory>

class Spawner {
public:
    Spawner(float mapSize);

    void update(float dt, const glm::vec3& playerPos,
        std::vector<std::unique_ptr<GameObject>>& mobs);

    int targetCount() const { return _targetCount; }
    int killCount() const { return _killCount; }
    void addKill() { _killCount++; }

private:
    float _mapSize;
    float _timer = 0;
    int _killCount = 0;
    int _targetCount = 15;

    struct MobWeights { float goblin, archer, ogre; };
    MobWeights _weights{0.5f, 0.3f, 0.2f};

    void spawnOne(std::vector<std::unique_ptr<GameObject>>& mobs);
    glm::vec3 randomEdgePos() const;
};
