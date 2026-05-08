#include "mob/Spawner.h"
#include <glm/gtc/random.hpp>

Spawner::Spawner(float mapSize) : _mapSize(mapSize) {}

void Spawner::update(float dt, const glm::vec3& playerPos,
    std::vector<std::unique_ptr<GameObject>>& mobs) {

    _timer -= dt;

    int alive = 0;
    for (auto& m : mobs)
        if (m->health.alive()) alive++;

    if (alive < _targetCount && _timer <= 0) {
        spawnOne(mobs);
        _timer = glm::linearRand(5.0f, 10.0f);
    }
}

void Spawner::spawnOne(std::vector<std::unique_ptr<GameObject>>& mobs) {
    glm::vec3 pos = randomEdgePos();
    float roll = glm::linearRand(0.0f, 1.0f);

    if (roll < _weights.goblin)
        mobs.push_back(std::make_unique<Goblin>(pos));
    else if (roll < _weights.goblin + _weights.archer)
        mobs.push_back(std::make_unique<Archer>(pos));
    else
        mobs.push_back(std::make_unique<Ogre>(pos));
}

glm::vec3 Spawner::randomEdgePos() const {
    float margin = _mapSize * 0.05f;
    int edge = rand() % 4;
    float x, z;
    switch (edge) {
    case 0: x = margin; z = glm::linearRand(margin, _mapSize - margin); break;
    case 1: x = _mapSize - margin; z = glm::linearRand(margin, _mapSize - margin); break;
    case 2: x = glm::linearRand(margin, _mapSize - margin); z = margin; break;
    case 3: x = glm::linearRand(margin, _mapSize - margin); z = _mapSize - margin; break;
    }
    return {x, 0, z};
}
