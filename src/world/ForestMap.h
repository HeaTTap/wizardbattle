#pragma once
#include <glm/glm.hpp>
#include <vector>

struct TreePlacement {
    glm::vec3 position;
    float scale;
    float rotation;
};

struct RockPlacement {
    glm::vec3 position;
    float scale;
};

class ForestMap {
public:
    ForestMap(float mapSize, int treeCount, int rockCount);

    const std::vector<TreePlacement>& trees() const { return _trees; }
    const std::vector<RockPlacement>& rocks() const { return _rocks; }

    bool isInClearing(glm::vec2 pos) const;
    static constexpr float CLEARING_SIZE = 40.0f;

private:
    std::vector<TreePlacement> _trees;
    std::vector<RockPlacement> _rocks;
    float _mapSize;
    glm::vec2 _center;
};
