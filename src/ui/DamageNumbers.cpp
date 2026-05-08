#include "ui/DamageNumbers.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

DamageNumbers::DamageNumbers() {
    _particleShader = new Shader("assets/shaders/default.vert",
        "assets/shaders/default.frag");
    _quad = new Mesh(Mesh::createQuad());
}

DamageNumbers::~DamageNumbers() {
    delete _particleShader;
    delete _quad;
}

void DamageNumbers::add(const glm::vec3& pos, int amount) {
    _numbers.push_back({pos, amount, 1.0f, 1.0f});
}

void DamageNumbers::update(float dt) {
    for (auto& n : _numbers) {
        n.life -= dt;
        n.worldPos.y += dt * 3.0f;
    }
    _numbers.erase(
        std::remove_if(_numbers.begin(), _numbers.end(),
            [](auto& n) { return n.life <= 0; }),
        _numbers.end());
}

void DamageNumbers::render(const glm::mat4& view, const glm::mat4& proj) {
    for (auto& n : _numbers) {
        _particleShader->use();
        _particleShader->setMat4("uView", &view[0][0]);
        _particleShader->setMat4("uProj", &proj[0][0]);
        glm::mat4 model = glm::translate(glm::mat4(1), n.worldPos)
            * glm::scale(glm::mat4(1), glm::vec3(1.0f));
        _particleShader->setMat4("uModel", &model[0][0]);
        _particleShader->setVec3("uLightColor", 1, 0.8f, 0.2f);
        _particleShader->setVec3("uAmbient", 1, 0.8f, 0.2f);
        _quad->draw();
    }
}
