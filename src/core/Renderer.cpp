#include "core/Renderer.h"
#include <GL/glew.h>

Renderer::Renderer()
    : _shader("assets/shaders/default.vert", "assets/shaders/default.frag") {

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Renderer::begin(const Camera& cam) {
    _cam = &cam;
}

void Renderer::end() {
    _cam = nullptr;
}

void Renderer::clear() {
    glClearColor(0.4f, 0.55f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawMesh(const Mesh& mesh, const Texture& tex,
    const glm::mat4& model, const glm::vec3& tint) {

    if (!_cam) return;
    _shader.use();
    _shader.setMat4("uModel", &model[0][0]);
    _shader.setMat4("uView", &_cam->view()[0][0]);
    _shader.setMat4("uProj", &_cam->proj()[0][0]);
    _shader.setVec3("uLightDir", -0.5f, -1.0f, -0.3f);
    _shader.setVec3("uLightColor", 1.0f, 0.95f, 0.8f);
    _shader.setVec3("uAmbient", 0.25f, 0.3f, 0.2f);
    _shader.setVec3("uViewPos", _cam->position().x,
        _cam->position().y, _cam->position().z);
    _shader.setVec3("uFogColor", 0.4f, 0.55f, 0.7f);
    _shader.setFloat("uFogStart", 50.0f);
    _shader.setFloat("uFogEnd", 120.0f);
    tex.bind(0);
    _shader.setInt("uTexture", 0);
    mesh.draw();
}
