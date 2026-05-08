#pragma once
#include "core/Shader.h"
#include "core/Camera.h"
#include "core/Mesh.h"
#include "core/Texture.h"
#include <glm/glm.hpp>

class Renderer {
public:
    Renderer();
    void begin(const Camera& cam);
    void end();
    void drawMesh(const Mesh& mesh, const Texture& tex,
        const glm::mat4& model, const glm::vec3& tint = {1,1,1});
    void clear();

    Shader& defaultShader() { return _shader; }

private:
    Shader _shader;
    const Camera* _cam = nullptr;
};
