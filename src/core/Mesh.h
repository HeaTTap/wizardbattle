#pragma once
#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class Mesh {
public:
    Mesh() = default;
    Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned>& idx);
    ~Mesh();

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void draw() const;
    bool valid() const { return _vao != 0; }

    static Mesh createQuad();
    static Mesh createCube();
    static Mesh fromOBJ(const char* path);

private:
    GLuint _vao = 0, _vbo = 0, _ebo = 0;
    unsigned _indexCount = 0;
};
