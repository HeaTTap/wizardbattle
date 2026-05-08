#include "world/Terrain.h"
#include <cmath>
#include <algorithm>
#include <glm/gtc/noise.hpp>

Terrain::Terrain(int gridSize, float cellSize)
    : _gridSize(gridSize), _cellSize(cellSize) {

    int vertCount = (gridSize + 1) * (gridSize + 1);
    _heights.resize(vertCount);

    std::vector<Vertex> verts(vertCount);
    for (int z = 0; z <= gridSize; z++) {
        for (int x = 0; x <= gridSize; x++) {
            int i = z * (gridSize + 1) + x;
            float wx = x * cellSize;
            float wz = z * cellSize;
            float h = perlinNoise(wx * 0.05f, wz * 0.05f, 4, 0.5f) * 5.0f;
            _heights[i] = h;

            verts[i].position = glm::vec3(wx, h, wz);
            verts[i].normal = glm::vec3(0, 1, 0);
            verts[i].texCoord = glm::vec2(wx * 0.1f, wz * 0.1f);
        }
    }

    std::vector<unsigned> idx;
    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {
            unsigned tl = z * (gridSize + 1) + x;
            unsigned tr = tl + 1;
            unsigned bl = (z + 1) * (gridSize + 1) + x;
            unsigned br = bl + 1;
            idx.insert(idx.end(), {tl, bl, br, tl, br, tr});
        }
    }

    for (size_t i = 0; i < idx.size(); i += 3) {
        glm::vec3 a = verts[idx[i]].position;
        glm::vec3 b = verts[idx[i+1]].position;
        glm::vec3 c = verts[idx[i+2]].position;
        glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
        verts[idx[i]].normal = n;
        verts[idx[i+1]].normal = n;
        verts[idx[i+2]].normal = n;
    }

    _mesh = Mesh(verts, idx);
}

float Terrain::heightAt(float x, float z) const {
    float gx = x / _cellSize;
    float gz = z / _cellSize;
    int ix = (int)glm::clamp(gx, 0.0f, (float)_gridSize);
    int iz = (int)glm::clamp(gz, 0.0f, (float)_gridSize);
    return _heights[iz * (_gridSize + 1) + ix];
}

float Terrain::perlinNoise(float x, float z, int octaves, float persistence) const {
    float val = 0;
    float amp = 1;
    float freq = 1;
    float maxVal = 0;
    for (int i = 0; i < octaves; i++) {
        val += glm::perlin(glm::vec2(x * freq, z * freq)) * amp;
        maxVal += amp;
        amp *= persistence;
        freq *= 2;
    }
    return val / maxVal;
}
