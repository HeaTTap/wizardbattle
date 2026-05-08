#include "world/ForestMap.h"
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

ForestMap::ForestMap(float mapSize, int treeCount, int rockCount)
    : _mapSize(mapSize), _center(mapSize/2, mapSize/2) {

    float margin = mapSize * 0.05f;
    _trees.reserve(treeCount);
    for (int i = 0; i < treeCount; i++) {
        glm::vec2 pos;
        int attempts = 0;
        do {
            pos = {glm::linearRand(margin, mapSize - margin),
                   glm::linearRand(margin, mapSize - margin)};
            attempts++;
        } while (isInClearing(pos) && attempts < 50);

        float scale = glm::linearRand(0.8f, 1.5f);
        float rot = glm::linearRand(0.0f, glm::pi<float>() * 2);
        _trees.push_back({glm::vec3(pos.x, 0, pos.y), scale, rot});
    }

    _rocks.reserve(rockCount);
    for (int i = 0; i < rockCount; i++) {
        glm::vec2 pos;
        int attempts = 0;
        do {
            pos = {glm::linearRand(margin, mapSize - margin),
                   glm::linearRand(margin, mapSize - margin)};
            attempts++;
        } while (isInClearing(pos) && attempts < 50);

        float scale = glm::linearRand(0.5f, 1.5f);
        _rocks.push_back({glm::vec3(pos.x, 0, pos.y), scale});
    }
}

bool ForestMap::isInClearing(glm::vec2 pos) const {
    return glm::distance(pos, _center) < CLEARING_SIZE / 2.0f;
}
