#include "core/Mesh.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cmath>

Mesh::Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned>& idx)
    : _indexCount(static_cast<unsigned>(idx.size())) {

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex),
        verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned),
        idx.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

Mesh::~Mesh() {
    if (_ebo) glDeleteBuffers(1, &_ebo);
    if (_vbo) glDeleteBuffers(1, &_vbo);
    if (_vao) glDeleteVertexArrays(1, &_vao);
}

Mesh::Mesh(Mesh&& other) noexcept
    : _vao(other._vao), _vbo(other._vbo), _ebo(other._ebo),
      _indexCount(other._indexCount) {
    other._vao = 0; other._vbo = 0; other._ebo = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        if (_ebo) glDeleteBuffers(1, &_ebo);
        if (_vbo) glDeleteBuffers(1, &_vbo);
        if (_vao) glDeleteVertexArrays(1, &_vao);
        _vao = other._vao; _vbo = other._vbo; _ebo = other._ebo;
        _indexCount = other._indexCount;
        other._vao = 0; other._vbo = 0; other._ebo = 0;
    }
    return *this;
}

void Mesh::draw() const {
    if (!_vao) return;
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

Mesh Mesh::createQuad() {
    std::vector<Vertex> verts = {
        {glm::vec3(-0.5f, -0.5f, 0), glm::vec3(0,0,1), glm::vec2(0,0)},
        {glm::vec3( 0.5f, -0.5f, 0), glm::vec3(0,0,1), glm::vec2(1,0)},
        {glm::vec3( 0.5f,  0.5f, 0), glm::vec3(0,0,1), glm::vec2(1,1)},
        {glm::vec3(-0.5f,  0.5f, 0), glm::vec3(0,0,1), glm::vec2(0,1)},
    };
    std::vector<unsigned> idx = {0,1,2, 0,2,3};
    return Mesh(verts, idx);
}

Mesh Mesh::createCube() {
    // 24 unique vertices (4 per face, 6 faces)
    struct FaceData { glm::vec3 pos[4]; glm::vec3 norm; };
    float h = 0.5f;
    FaceData fds[6] = {
        {{glm::vec3(-h,-h, h),glm::vec3( h,-h, h),glm::vec3( h, h, h),glm::vec3(-h, h, h)}, glm::vec3(0,0,1)},
        {{glm::vec3( h,-h,-h),glm::vec3(-h,-h,-h),glm::vec3(-h, h,-h),glm::vec3( h, h,-h)}, glm::vec3(0,0,-1)},
        {{glm::vec3(-h,-h,-h),glm::vec3(-h,-h, h),glm::vec3(-h, h, h),glm::vec3(-h, h,-h)}, glm::vec3(-1,0,0)},
        {{glm::vec3( h,-h, h),glm::vec3( h,-h,-h),glm::vec3( h, h,-h),glm::vec3( h, h, h)}, glm::vec3(1,0,0)},
        {{glm::vec3(-h, h, h),glm::vec3( h, h, h),glm::vec3( h, h,-h),glm::vec3(-h, h,-h)}, glm::vec3(0,1,0)},
        {{glm::vec3(-h,-h,-h),glm::vec3( h,-h,-h),glm::vec3( h,-h, h),glm::vec3(-h,-h, h)}, glm::vec3(0,-1,0)},
    };

    std::vector<Vertex> verts;
    std::vector<unsigned> idx;
    for (int f = 0; f < 6; f++) {
        for (int v = 0; v < 4; v++) {
            verts.push_back({fds[f].pos[v], fds[f].norm,
                glm::vec2(v==0||v==3?0.0f:1.0f, v==0||v==1?0.0f:1.0f)});
        }
        unsigned base = f * 4;
        idx.insert(idx.end(), {base, base+1, base+2, base, base+2, base+3});
    }
    return Mesh(verts, idx);
}

Mesh Mesh::fromOBJ(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        fprintf(stderr, "Cannot open OBJ: %s\n", path);
        return Mesh();
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<Vertex> verts;
    std::vector<unsigned> indices;
    std::unordered_map<std::string, unsigned> indexMap;

    auto hash = [](int p, int t, int n) {
        return std::to_string(p) + "/" + std::to_string(t) + "/" + std::to_string(n);
    };

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            positions.push_back({x, y, z});
        } else if (type == "vt") {
            float u, v;
            ss >> u >> v;
            texcoords.push_back({u, v});
        } else if (type == "vn") {
            float x, y, z;
            ss >> x >> y >> z;
            normals.push_back({x, y, z});
        } else if (type == "f") {
            std::string v[3];
            ss >> v[0] >> v[1] >> v[2];
            for (int i = 0; i < 3; i++) {
                int p = 0, t = 0, n = 0;
                sscanf(v[i].c_str(), "%d/%d/%d", &p, &t, &n);
                if (p < 0) p += (int)positions.size() + 1;
                if (t < 0) t += (int)texcoords.size() + 1;
                if (n < 0) n += (int)normals.size() + 1;

                std::string key = hash(p, t, n);
                auto it = indexMap.find(key);
                if (it != indexMap.end()) {
                    indices.push_back(it->second);
                } else {
                    Vertex vtx;
                    vtx.position = positions[p - 1];
                    vtx.texCoord = (t > 0) ? texcoords[t - 1] : glm::vec2(0);
                    glm::vec3 rawN = (n > 0) ? normals[n - 1] : glm::vec3(0,1,0);
                    vtx.normal = rawN;
                    unsigned idx = (unsigned)verts.size();
                    verts.push_back(vtx);
                    indices.push_back(idx);
                    indexMap[key] = idx;
                }
            }
        }
    }

    if (normals.empty()) {
        for (size_t i = 0; i < indices.size(); i += 3) {
            glm::vec3& a = verts[indices[i]].position;
            glm::vec3& b = verts[indices[i+1]].position;
            glm::vec3& c = verts[indices[i+2]].position;
            glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
            verts[indices[i]].normal = n;
            verts[indices[i+1]].normal = n;
            verts[indices[i+2]].normal = n;
        }
    }

    return Mesh(verts, indices);
}
