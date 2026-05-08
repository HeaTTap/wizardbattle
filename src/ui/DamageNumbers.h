#pragma once
#include "core/Mesh.h"
#include "core/Shader.h"
#include <glm/glm.hpp>
#include <vector>

struct DamageFloat {
    glm::vec3 worldPos;
    int amount;
    float life = 1.0f;
    float maxLife = 1.0f;
};

class DamageNumbers {
public:
    DamageNumbers();
    ~DamageNumbers();

    void add(const glm::vec3& pos, int amount);
    void update(float dt);
    void render(const glm::mat4& view, const glm::mat4& proj);

private:
    std::vector<DamageFloat> _numbers;
    Shader* _particleShader = nullptr;
    Mesh* _quad = nullptr;
};
