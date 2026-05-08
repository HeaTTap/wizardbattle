#include "entity/Collision.h"
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

bool aabbIntersects(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

AABB getWorldAABB(const GameObject& obj) {
    glm::vec3 half = obj.collider.halfExtents;
    glm::vec3 pos = obj.transform.position;
    return {pos - half, pos + half};
}

SpatialGrid::SpatialGrid(float worldSize, float cellSize)
    : _worldSize(worldSize), _cellSize(cellSize),
      _cellsPerSide((int)ceil(worldSize / cellSize)),
      _walkable(_cellsPerSide * _cellsPerSide, true) {}

void SpatialGrid::clear() {
    _cells.clear();
}

void SpatialGrid::insert(GameObject* obj) {
    AABB box = getWorldAABB(*obj);
    int cx0 = (int)(box.min.x / _cellSize);
    int cz0 = (int)(box.min.z / _cellSize);
    int cx1 = (int)(box.max.x / _cellSize);
    int cz1 = (int)(box.max.z / _cellSize);

    for (int cz = cz0; cz <= cz1; cz++)
        for (int cx = cx0; cx <= cx1; cx++)
            _cells[cellKey(cx, cz)].push_back(obj);
}

std::vector<GameObject*> SpatialGrid::query(const glm::vec3& pos, float radius) {
    std::vector<GameObject*> result;
    int cx0 = (int)((pos.x - radius) / _cellSize);
    int cz0 = (int)((pos.z - radius) / _cellSize);
    int cx1 = (int)((pos.x + radius) / _cellSize);
    int cz1 = (int)((pos.z + radius) / _cellSize);

    for (int cz = cz0; cz <= cz1; cz++)
        for (int cx = cx0; cx <= cx1; cx++) {
            auto it = _cells.find(cellKey(cx, cz));
            if (it != _cells.end())
                result.insert(result.end(), it->second.begin(), it->second.end());
        }
    return result;
}

int SpatialGrid::cellKey(int cx, int cy) const {
    return cy * _cellsPerSide + cx;
}

bool SpatialGrid::isWalkable(int cx, int cy) const {
    if (cx < 0 || cx >= _cellsPerSide || cy < 0 || cy >= _cellsPerSide)
        return false;
    return _walkable[cy * _cellsPerSide + cx];
}

void SpatialGrid::setWalkable(int cx, int cy, bool w) {
    if (cx >= 0 && cx < _cellsPerSide && cy >= 0 && cy < _cellsPerSide)
        _walkable[cy * _cellsPerSide + cx] = w;
}

std::vector<glm::ivec2> SpatialGrid::getNeighbors(int cx, int cy) const {
    std::vector<glm::ivec2> n;
    const int dx[] = {-1,1,0,0,-1,-1,1,1};
    const int dy[] = {0,0,-1,1,-1,1,-1,1};
    for (int i = 0; i < 8; i++) {
        int nx = cx + dx[i], ny = cy + dy[i];
        if (isWalkable(nx, ny))
            n.push_back({nx, ny});
    }
    return n;
}

struct PathNode {
    glm::ivec2 pos;
    float g, f;
    glm::ivec2 parent{-1,-1};
    bool operator>(const PathNode& o) const { return f > o.f; }
};

std::vector<glm::vec2> findPath(const SpatialGrid& grid,
    glm::vec2 start, glm::vec2 end) {

    int cs = (int)ceil(grid.cellSize());
    glm::ivec2 sc((int)(start.x / cs), (int)(start.y / cs));
    glm::ivec2 ec((int)(end.x / cs), (int)(end.y / cs));

    std::priority_queue<PathNode, std::vector<PathNode>,
        std::greater<PathNode>> open;
    std::unordered_map<int, PathNode> visited;

    auto key = [&](const glm::ivec2& p) { return p.y * 10000 + p.x; };

    PathNode startNode{sc, 0,
        glm::length(glm::vec2(ec - sc)), {-1,-1}};
    startNode.f = startNode.g + glm::length(glm::vec2(ec - sc));
    open.push(startNode);
    visited[key(sc)] = startNode;

    while (!open.empty()) {
        PathNode cur = open.top(); open.pop();
        if (cur.pos == ec) {
            std::vector<glm::vec2> path;
            glm::ivec2 p = ec;
            while (p != sc) {
                path.push_back(glm::vec2(p.x * cs + cs/2.0f, p.y * cs + cs/2.0f));
                auto it = visited.find(key(p));
                if (it == visited.end() || it->second.parent == glm::ivec2(-1)) break;
                p = it->second.parent;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (auto& nb : grid.getNeighbors(cur.pos.x, cur.pos.y)) {
            float moveCost = (nb.x != cur.pos.x && nb.y != cur.pos.y) ? 1.414f : 1.0f;
            float ng = cur.g + moveCost;
            int nk = key(nb);
            auto it = visited.find(nk);
            if (it == visited.end() || ng < it->second.g) {
                PathNode nn{nb, ng, 0, cur.pos};
                nn.f = ng + glm::length(glm::vec2(ec - nb));
                open.push(nn);
                visited[nk] = nn;
            }
        }
    }
    return {};
}
