#pragma once
#include "core/Mesh.h"
#include <glm/glm.hpp>
#include <vector>

class Terrain {
public:
    Terrain(int gridSize, float cellSize);

    void draw() const { _mesh.draw(); }
    float heightAt(float x, float z) const;
    float size() const { return _gridSize * _cellSize; }
    const Mesh& mesh() const { return _mesh; }

private:
    int _gridSize;
    float _cellSize;
    Mesh _mesh;
    std::vector<float> _heights;

    float perlinNoise(float x, float z, int octaves, float persistence) const;
};
