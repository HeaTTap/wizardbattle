#pragma once
#include "entity/GameObject.h"
#include <vector>
#include <unordered_map>

struct AABB {
    glm::vec3 min, max;
};

class SpatialGrid {
public:
    SpatialGrid(float worldSize, float cellSize);

    void clear();
    void insert(GameObject* obj);
    std::vector<GameObject*> query(const glm::vec3& pos, float radius);

    std::vector<glm::ivec2> getNeighbors(int cx, int cy) const;
    bool isWalkable(int cx, int cy) const;
    void setWalkable(int cx, int cy, bool w);
    float cellSize() const { return _cellSize; }
    float worldSize() const { return _worldSize; }

private:
    float _worldSize;
    float _cellSize;
    int _cellsPerSide;
    std::unordered_map<int, std::vector<GameObject*>> _cells;
    std::vector<bool> _walkable;

    int cellKey(int cx, int cy) const;
};

bool aabbIntersects(const AABB& a, const AABB& b);
AABB getWorldAABB(const GameObject& obj);

std::vector<glm::vec2> findPath(const SpatialGrid& grid,
    glm::vec2 start, glm::vec2 end);
