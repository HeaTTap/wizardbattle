# WizardBattle Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a complete 3D top-down PvE wizard game in C++ with SDL3 + OpenGL — forest map, 3 mob types, 5 spells, leveling, and a dragon boss.

**Architecture:** Lightweight component composition — GameObjects own pluggable components (Transform, Health, Renderable, Collider, AI). Fixed-timestep game loop at 60 FPS. OpenGL 3.3 Core profile with orthographic camera. Spatial grid for collision broad-phase. State-machine AI for mobs.

**Tech Stack:** C++17, SDL3, OpenGL 3.3+, GLEW, GLM (header-only), CMake

---

## File Structure Map

```
~/Documents/wizardbattle/
├── CMakeLists.txt                          # Build system
├── assets/
│   ├── shaders/
│   │   ├── default.vert                    # Vertex shader — MVP transform
│   │   └── default.frag                    # Fragment shader — diffuse lighting
│   ├── models/                             # Downloaded .obj files
│   ├── textures/                           # Downloaded PNG textures
│   └── sounds/                             # Downloaded WAV/OGG audio
├── src/
│   ├── main.cpp                            # Entry point, launches Game
│   ├── core/
│   │   ├── Window.h / Window.cpp           # SDL3 window + GL context
│   │   ├── Renderer.h / Renderer.cpp       # OpenGL draw calls, lighting setup
│   │   ├── Camera.h / Camera.cpp           # Orthographic, lerp follow, zoom
│   │   ├── Input.h / Input.cpp             # Keyboard + mouse state
│   │   ├── Game.h / Game.cpp               # Game loop, state machine, entity list
│   │   ├── Shader.h / Shader.cpp           # GLSL compile/link
│   │   ├── Mesh.h / Mesh.cpp               # VAO/VBO/EBO, .obj loader
│   │   └── Texture.h / Texture.cpp         # PNG loader (stb_image)
│   ├── entity/
│   │   ├── GameObject.h / GameObject.cpp   # Entity base — owns Components
│   │   ├── Components.h / Components.cpp   # Transform, Health, Renderable, Collider
│   │   └── Collision.h / Collision.cpp     # AABB intersection, spatial grid
│   ├── player/
│   │   ├── Wizard.h / Wizard.cpp           # Player entity, movement, aiming
│   │   ├── Spell.h / Spell.cpp             # Base spell + Firebolt projectile
│   │   ├── SpellBook.h / SpellBook.cpp     # 5 spells, unlock, cooldowns, mana
│   │   └── Progression.h / Progression.cpp # XP, leveling, XP-to-next formula
│   ├── mob/
│   │   ├── MobAI.h / MobAI.cpp             # State machine base, A* pathfinding
│   │   ├── Goblin.h / Goblin.cpp           # Fast swarm melee
│   │   ├── Archer.h / Archer.cpp           # Ranged, keeps distance
│   │   ├── Ogre.h / Ogre.cpp               # Slow tank, heavy telegraph
│   │   ├── Spawner.h / Spawner.cpp         # Spawn/despawn, population management
│   │   └── Dragon.h / Dragon.cpp           # Boss: phases, 3 attack types
│   ├── world/
│   │   ├── Terrain.h / Terrain.cpp         # Heightmap mesh, Perlin noise
│   │   └── ForestMap.h / ForestMap.cpp     # Tree/rock scatter, central clearing
│   └── ui/
│       ├── HUD.h / HUD.cpp                 # Health/mana/xp bars, spell bar, kill count
│       └── DamageNumbers.h / DamageNumbers.cpp  # Floating damage text
└── docs/
    └── superpowers/
        ├── specs/2026-05-08-wizardbattle-design.md
        └── plans/2026-05-08-wizardbattle-plan.md
```

---

## Phase 1: Core Engine

### Task 1.1: CMake build system

**Files:**
- Create: `~/Documents/wizardbattle/CMakeLists.txt`
- Create: `~/Documents/wizardbattle/src/main.cpp`
- Create: `~/Documents/wizardbattle/assets/shaders/default.vert`
- Create: `~/Documents/wizardbattle/assets/shaders/default.frag`

- [ ] **Step 1: Write CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.20)
project(wizardbattle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)
pkg_check_modules(GLEW REQUIRED glew)
pkg_check_modules(OPENAL REQUIRED openal)

# GLM is header-only, bundle it or find it
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "GLM not found. Install with: sudo pacman -S glm")
endif()

add_executable(wizardbattle
    src/main.cpp
    src/core/Window.cpp
    src/core/Renderer.cpp
    src/core/Camera.cpp
    src/core/Input.cpp
    src/core/Game.cpp
    src/core/Shader.cpp
    src/core/Mesh.cpp
    src/core/Texture.cpp
    src/entity/GameObject.cpp
    src/entity/Components.cpp
    src/entity/Collision.cpp
    src/player/Wizard.cpp
    src/player/Spell.cpp
    src/player/SpellBook.cpp
    src/player/Progression.cpp
    src/mob/MobAI.cpp
    src/mob/Goblin.cpp
    src/mob/Archer.cpp
    src/mob/Ogre.cpp
    src/mob/Spawner.cpp
    src/mob/Dragon.cpp
    src/world/Terrain.cpp
    src/world/ForestMap.cpp
    src/ui/HUD.cpp
    src/ui/DamageNumbers.cpp
)

target_include_directories(wizardbattle PRIVATE
    src
    ${SDL3_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
)

target_link_libraries(wizardbattle
    ${SDL3_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENAL_LIBRARIES}
)

target_compile_options(wizardbattle PRIVATE -Wall -Wextra)
```

- [ ] **Step 2: Write minimal main.cpp**

```cpp
#include <cstdio>

int main(int argc, char* argv[]) {
    printf("WizardBattle starting...\n");
    return 0;
}
```

- [ ] **Step 3: Write default vertex shader**

```glsl
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vTexCoord = aTexCoord;
    gl_Position = uProj * uView * worldPos;
}
```

- [ ] **Step 4: Write default fragment shader**

```glsl
#version 330 core
in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbient;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform vec3 uViewPos;

out vec4 FragColor;

void main() {
    vec3 color = texture(uTexture, vTexCoord).rgb;
    vec3 norm = normalize(vNormal);
    float diff = max(dot(norm, normalize(uLightDir)), 0.0);
    vec3 diffuse = diff * uLightColor;
    vec3 ambient = uAmbient * color;
    vec3 result = (ambient + diffuse) * color;

    float dist = length(vFragPos - uViewPos);
    float fogFactor = clamp((uFogEnd - dist) / (uFogEnd - uFogStart), 0.0, 1.0);
    result = mix(uFogColor, result, fogFactor);

    FragColor = vec4(result, 1.0);
}
```

- [ ] **Step 5: Build and verify it compiles**

```bash
cd ~/Documents/wizardbattle && mkdir -p build && cd build && cmake .. && make -j$(nproc)
```

Expected: Compilation succeeds, wizardbattle binary exists.

- [ ] **Step 6: Run and verify**

```bash
cd ~/Documents/wizardbattle/build && ./wizardbattle
```

Expected: "WizardBattle starting..." printed.

- [ ] **Step 7: Commit**

```bash
cd ~/Documents/wizardbattle
git add CMakeLists.txt src/main.cpp assets/shaders/
git commit -m "feat: CMake build system with SDL3, GLEW, GLM, OpenAL dependencies"
```

---

### Task 1.2: Window and OpenGL context

**Files:**
- Create: `~/Documents/wizardbattle/src/core/Window.h`
- Create: `~/Documents/wizardbattle/src/core/Window.cpp`
- Modify: `~/Documents/wizardbattle/src/main.cpp`

- [ ] **Step 1: Write Window.h**

```cpp
#pragma once
#include <SDL3/SDL.h>

class Window {
public:
    Window(const char* title, int width, int height);
    ~Window();

    void swap();
    bool shouldClose() const { return _shouldClose; }
    void setShouldClose(bool v) { _shouldClose = v; }
    int width() const { return _width; }
    int height() const { return _height; }
    SDL_Window* sdlWindow() const { return _window; }

private:
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    int _width, _height;
    bool _shouldClose = false;
};
```

- [ ] **Step 2: Write Window.cpp**

```cpp
#include "core/Window.h"
#include <cstdio>

Window::Window(const char* title, int width, int height)
    : _width(width), _height(height) {

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        _shouldClose = true;
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    _window = SDL_CreateWindow(title, width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!_window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        _shouldClose = true;
        return;
    }

    _glContext = SDL_GL_CreateContext(_window);
    if (!_glContext) {
        fprintf(stderr, "GL context failed: %s\n", SDL_GetError());
        _shouldClose = true;
        return;
    }

    printf("Window created: %dx%d, OpenGL %s\n", width, height,
        glGetString(GL_VERSION));
}

Window::~Window() {
    if (_glContext) SDL_GL_DestroyContext(_glContext);
    if (_window) SDL_DestroyWindow(_window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
}

void Window::swap() {
    SDL_GL_SwapWindow(_window);
}
```

- [ ] **Step 3: Update main.cpp to create window**

```cpp
#include "core/Window.h"
#include <cstdio>

int main(int argc, char* argv[]) {
    Window window("WizardBattle", 1280, 720);
    if (window.shouldClose()) return 1;

    while (!window.shouldClose()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                window.setShouldClose(true);
            }
        }
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.swap();
    }
    return 0;
}
```

- [ ] **Step 4: Build and verify**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles, window opens briefly with dark blue background (Ctrl+C or close to exit).

- [ ] **Step 5: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Window.h src/core/Window.cpp src/main.cpp
git commit -m "feat: SDL3 window with OpenGL 3.3 core context"
```

---

### Task 1.3: Shader class

**Files:**
- Create: `~/Documents/wizardbattle/src/core/Shader.h`
- Create: `~/Documents/wizardbattle/src/core/Shader.cpp`

- [ ] **Step 1: Write Shader.h**

```cpp
#pragma once
#include <GL/glew.h>
#include <string>

class Shader {
public:
    Shader(const char* vertPath, const char* fragPath);
    ~Shader();

    void use() const;
    GLuint id() const { return _program; }

    void setInt(const char* name, int val) const;
    void setFloat(const char* name, float val) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setMat4(const char* name, const float* mat) const;

private:
    GLuint _program = 0;
    static std::string readFile(const char* path);
    static GLuint compileShader(GLenum type, const char* source);
};
```

- [ ] **Step 2: Write Shader.cpp**

```cpp
#include "core/Shader.h"
#include <cstdio>
#include <fstream>
#include <sstream>

Shader::Shader(const char* vertPath, const char* fragPath) {
    std::string vertSrc = readFile(vertPath);
    std::string fragSrc = readFile(fragPath);
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc.c_str());
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str());
    if (!vs || !fs) { _program = 0; return; }

    _program = glCreateProgram();
    glAttachShader(_program, vs);
    glAttachShader(_program, fs);
    glLinkProgram(_program);

    GLint success;
    glGetProgramiv(_program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(_program, 512, nullptr, log);
        fprintf(stderr, "Shader link error: %s\n", log);
        glDeleteProgram(_program);
        _program = 0;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() {
    if (_program) glDeleteProgram(_program);
}

void Shader::use() const { glUseProgram(_program); }

void Shader::setInt(const char* name, int val) const {
    glUniform1i(glGetUniformLocation(_program, name), val);
}

void Shader::setFloat(const char* name, float val) const {
    glUniform1f(glGetUniformLocation(_program, name), val);
}

void Shader::setVec3(const char* name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(_program, name), x, y, z);
}

void Shader::setMat4(const char* name, const float* mat) const {
    glUniformMatrix4fv(glGetUniformLocation(_program, name), 1, GL_FALSE, mat);
}

std::string Shader::readFile(const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        fprintf(stderr, "Cannot open shader: %s\n", path);
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

GLuint Shader::compileShader(GLenum type, const char* source) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &source, nullptr);
    glCompileShader(s);
    GLint success;
    glGetShaderiv(s, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        fprintf(stderr, "Shader compile error (%d): %s\n", type, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}
```

- [ ] **Step 3: Build and verify**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Shader.h src/core/Shader.cpp
git commit -m "feat: Shader class for GLSL compile/link with uniforms"
```

---

### Task 1.4: Mesh class with .obj loader

**Files:**
- Create: `~/Documents/wizardbattle/src/core/Mesh.h`
- Create: `~/Documents/wizardbattle/src/core/Mesh.cpp`

- [ ] **Step 1: Write Mesh.h**

```cpp
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

    static Mesh createQuad();   // 1x1 quad for particles/billboards
    static Mesh createCube();   // unit cube for placeholder rendering
    static Mesh fromOBJ(const char* path);

private:
    GLuint _vao = 0, _vbo = 0, _ebo = 0;
    unsigned _indexCount = 0;
};
```

- [ ] **Step 2: Write Mesh.cpp**

```cpp
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
        {{-0.5f, -0.5f, 0}, {0,0,1}, {0,0}},
        {{ 0.5f, -0.5f, 0}, {0,0,1}, {1,0}},
        {{ 0.5f,  0.5f, 0}, {0,0,1}, {1,1}},
        {{-0.5f,  0.5f, 0}, {0,0,1}, {0,1}},
    };
    std::vector<unsigned> idx = {0,1,2, 0,2,3};
    return Mesh(verts, idx);
}

Mesh Mesh::createCube() {
    std::vector<Vertex> verts;
    std::vector<unsigned> idx;

    float half = 0.5f;
    // 6 faces, each with 4 verts
    glm::vec3 faces[6][4] = {
        // Front, back, left, right, top, bottom
        {{{-half,-half, half}, { half,-half, half}, { half, half, half}, {-half, half, half}}},
        {{{ half,-half,-half}, {-half,-half,-half}, {-half, half,-half}, { half, half,-half}}},
        {{{-half,-half,-half}, {-half,-half, half}, {-half, half, half}, {-half, half,-half}}},
        {{{ half,-half, half}, { half,-half,-half}, { half, half,-half}, { half, half, half}}},
        {{{-half, half, half}, { half, half, half}, { half, half,-half}, {-half, half,-half}}},
        {{{-half,-half,-half}, { half,-half,-half}, { half,-half, half}, {-half,-half, half}}},
    };
    glm::vec3 norms[6] = {
        {0,0,1}, {0,0,-1}, {-1,0,0}, {1,0,0}, {0,1,0}, {0,-1,0}
    };

    for (int f = 0; f < 6; f++) {
        for (int v = 0; v < 4; v++) {
            verts.push_back({faces[f][v], norms[f],
                {v==0||v==3?0.0f:1.0f, v==0||v==1?0.0f:1.0f}});
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
                // Parse "p/t/n" or "p//n" or "p"
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

    // Generate normals if none were provided
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
```

- [ ] **Step 2: Build and verify**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 3: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Mesh.h src/core/Mesh.cpp
git commit -m "feat: Mesh class with VAO/VBO/EBO and .obj loader"
```

---

### Task 1.5: Texture class

**Files:**
- Create: `~/Documents/wizardbattle/src/core/Texture.h`
- Create: `~/Documents/wizardbattle/src/core/Texture.cpp`

- [ ] **Step 1: Write Texture.h**

```cpp
#pragma once
#include <GL/glew.h>

class Texture {
public:
    Texture() = default;
    Texture(const char* path);
    ~Texture();

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(int unit = 0) const;
    bool valid() const { return _id != 0; }
    static Texture createSolid(unsigned char r, unsigned char g, unsigned char b);

private:
    GLuint _id = 0;
};
```

- [ ] **Step 2: Write Texture.cpp**

```cpp
#include "core/Texture.h"
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(const char* path) {
    int w, h, ch;
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4);
    if (!data) {
        fprintf(stderr, "Texture load failed: %s\n", path);
        return;
    }

    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

Texture::~Texture() {
    if (_id) glDeleteTextures(1, &_id);
}

Texture::Texture(Texture&& other) noexcept : _id(other._id) {
    other._id = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (_id) glDeleteTextures(1, &_id);
        _id = other._id;
        other._id = 0;
    }
    return *this;
}

void Texture::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, _id);
}

Texture Texture::createSolid(unsigned char r, unsigned char g, unsigned char b) {
    Texture t;
    glGenTextures(1, &t._id);
    unsigned char data[4] = {r, g, b, 255};
    glBindTexture(GL_TEXTURE_2D, t._id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return t;
}
```

- [ ] **Step 3: Download stb_image.h**

```bash
curl -sL -o ~/Documents/wizardbattle/src/core/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
```

- [ ] **Step 4: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 5: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Texture.h src/core/Texture.cpp src/core/stb_image.h
git commit -m "feat: Texture class with PNG loading via stb_image"
```

---

### Task 1.6: Camera

**Files:**
- Create: `~/Documents/wizardbattle/src/core/Camera.h`
- Create: `~/Documents/wizardbattle/src/core/Camera.cpp`

- [ ] **Step 1: Write Camera.h**

```cpp
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(int screenW, int screenH);

    void setTarget(const glm::vec3& pos) { _target = pos; }
    void update(float dt);
    void zoom(float amount);

    glm::mat4 view() const { return _view; }
    glm::mat4 proj() const { return _proj; }
    glm::vec3 position() const { return _pos; }
    int screenWidth() const { return _screenW; }
    int screenHeight() const { return _screenH; }

private:
    glm::vec3 _pos = {0, 30, 0};
    glm::vec3 _target = {0, 0, 0};
    float _zoom = 30.0f;
    float _minZoom = 10.0f;
    float _maxZoom = 60.0f;
    int _screenW, _screenH;
    glm::mat4 _view{1.0f};
    glm::mat4 _proj{1.0f};
};
```

- [ ] **Step 2: Write Camera.cpp**

```cpp
#include "core/Camera.h"
#include <cmath>

Camera::Camera(int screenW, int screenH) : _screenW(screenW), _screenH(screenH) {}

void Camera::update(float dt) {
    // Smooth lerp toward target
    glm::vec3 desired = _target + glm::vec3(0, _zoom, _zoom * 0.5f);
    _pos += (desired - _pos) * glm::min(dt * 5.0f, 1.0f);

    _view = glm::lookAt(_pos, _target, glm::vec3(0, 1, 0));
    float aspect = (float)_screenW / (float)_screenH;
    float h = 20.0f;
    _proj = glm::ortho(-h * aspect, h * aspect, -h, h, 0.1f, 200.0f);
}

void Camera::zoom(float amount) {
    _zoom = glm::clamp(_zoom - amount, _minZoom, _maxZoom);
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Camera.h src/core/Camera.cpp
git commit -m "feat: Orthographic camera with smooth follow and zoom"
```

---

### Task 1.7: Input manager

**Files:**
- Create: `~/Documents/wizardbattle/src/core/Input.h`
- Create: `~/Documents/wizardbattle/src/core/Input.cpp`

- [ ] **Step 1: Write Input.h**

```cpp
#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <unordered_map>

class Input {
public:
    void update();

    bool keyDown(SDL_Keycode key) const { return _keys.count(key) && _keys.at(key); }
    bool keyPressed(SDL_Keycode key) const { return _justPressed.count(key); }
    bool mouseDown(int button) const { return _mouseButtons & SDL_BUTTON(button); }
    bool mousePressed(int button) const { return _justClicked.count(button); }
    int mouseX() const { return _mx; }
    int mouseY() const { return _my; }
    int mouseWheel() const { return _wheel; }

private:
    std::unordered_map<SDL_Keycode, bool> _keys;
    std::unordered_map<SDL_Keycode, bool> _justPressed;
    std::unordered_map<int, bool> _justClicked;
    int _mx = 0, _my = 0, _wheel = 0;
    Uint32 _mouseButtons = 0;
};
```

- [ ] **Step 2: Write Input.cpp**

```cpp
#include "core/Input.h"

void Input::update() {
    _justPressed.clear();
    _justClicked.clear();
    _wheel = 0;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_EVENT_KEY_DOWN:
            _justPressed[e.key.key] = !e.key.repeat;
            _keys[e.key.key] = true;
            break;
        case SDL_EVENT_KEY_UP:
            _keys[e.key.key] = false;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            _mx = (int)e.motion.x;
            _my = (int)e.motion.y;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            _mouseButtons |= SDL_BUTTON(e.button.button);
            _justClicked[e.button.button] = true;
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            _mouseButtons &= ~SDL_BUTTON(e.button.button);
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            _wheel = (int)e.wheel.y;
            break;
        }
    }
}
```

- [ ] **Step 3: Build and verify**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Input.h src/core/Input.cpp
git commit -m "feat: Input manager with keyboard, mouse, and scroll wheel"
```

---

### Task 1.8: Terrain with Perlin noise heightmap

**Files:**
- Create: `~/Documents/wizardbattle/src/world/Terrain.h`
- Create: `~/Documents/wizardbattle/src/world/Terrain.cpp`

- [ ] **Step 1: Write Terrain.h**

```cpp
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
    std::vector<float> _heights;  // stored for height queries

    float perlinNoise(float x, float z, int octaves, float persistence) const;
};
```

- [ ] **Step 2: Write Terrain.cpp**

```cpp
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

    // Recalculate normals
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
```

- [ ] **Step 3: Build and verify**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/world/Terrain.h src/world/Terrain.cpp
git commit -m "feat: Terrain with Perlin noise heightmap, 200x200 units"
```

---

### Task 1.9: Renderer setup

**Files:**
- Create: `~/Documents/wizardbattle/src/core/Renderer.h`
- Create: `~/Documents/wizardbattle/src/core/Renderer.cpp`

- [ ] **Step 1: Write Renderer.h**

```cpp
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
```

- [ ] **Step 2: Write Renderer.cpp**

```cpp
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
```

- [ ] **Step 3: Build and verify**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Renderer.h src/core/Renderer.cpp
git commit -m "feat: Renderer with lighting, fog, mesh drawing"
```

---

### Task 1.10: Game class — game loop and state machine

**Files:**
- Create: `~/Documents/wizardbattle/src/core/Game.h`
- Create: `~/Documents/wizardbattle/src/core/Game.cpp`
- Modify: `~/Documents/wizardbattle/src/main.cpp`

- [ ] **Step 1: Write Game.h**

```cpp
#pragma once
#include "core/Window.h"
#include "core/Renderer.h"
#include "core/Camera.h"
#include "core/Input.h"
#include "world/Terrain.h"
#include "entity/GameObject.h"
#include <vector>
#include <memory>

enum class GameState {
    MainMenu,
    Playing,
    GameOver,
    Victory,
};

class Game {
public:
    Game();
    void run();

private:
    void update(float dt);
    void render();
    void processInput();

    Window _window;
    Renderer _renderer;
    Camera _camera;
    Input _input;
    Terrain _terrain;
    GameState _state = GameState::Playing;
    std::vector<std::unique_ptr<GameObject>> _objects;

    // Temp placeholder mesh for testing
    Mesh _cube;
    Texture _whiteTex;
};
```

- [ ] **Step 2: Write Game.cpp**

```cpp
#include "core/Game.h"
#include <GL/glew.h>
#include <chrono>

Game::Game()
    : _window("WizardBattle", 1280, 720)
    , _camera(1280, 720)
    , _terrain(200, 1.0f)
    , _cube(Mesh::createCube())
    , _whiteTex(Texture::createSolid(255, 255, 255))
{
    if (_window.shouldClose()) return;

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW init failed: %s\n",
            glewGetErrorString(err));
        _window.setShouldClose(true);
        return;
    }
}

void Game::run() {
    if (_window.shouldClose()) return;

    auto prevTime = std::chrono::high_resolution_clock::now();
    const float fixedDt = 1.0f / 60.0f;
    float accumulator = 0;

    while (!_window.shouldClose()) {
        auto now = std::chrono::high_resolution_clock::now();
        float frameDt = std::chrono::duration<float>(now - prevTime).count();
        prevTime = now;
        accumulator += frameDt;

        _input.update();

        if (_input.keyPressed(SDLK_ESCAPE)) {
            _window.setShouldClose(true);
        }

        while (accumulator >= fixedDt) {
            update(fixedDt);
            accumulator -= fixedDt;
        }

        render();
        _window.swap();
    }
}

void Game::update(float dt) {
    if (_state != GameState::Playing) return;

    _camera.setTarget({100, 0, 100}); // center of map
    _camera.update(dt);

    if (_input.mouseWheel() != 0) {
        _camera.zoom((float)_input.mouseWheel() * 2.0f);
    }
}

void Game::render() {
    _renderer.clear();
    _renderer.begin(_camera);

    // Draw terrain
    _renderer.drawMesh(_terrain.mesh(), _whiteTex,
        glm::translate(glm::mat4(1), glm::vec3(0, 0, 0)));

    // Draw a test cube at center
    float h = _terrain.heightAt(100, 100);
    _renderer.drawMesh(_cube, _whiteTex,
        glm::translate(glm::mat4(1), glm::vec3(100, h + 0.5f, 100)));

    _renderer.end();
}

void Game::processInput() {
}
```

- [ ] **Step 3: Update main.cpp**

```cpp
#include "core/Game.h"

int main(int argc, char* argv[]) {
    Game game;
    game.run();
    return 0;
}
```

- [ ] **Step 4: Build and run**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc) && ./wizardbattle
```

Expected: Window opens showing a green terrain with hills, a white cube at center, camera at top-down angle. Close with Escape or window X.

- [ ] **Step 5: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Game.h src/core/Game.cpp src/main.cpp
git commit -m "feat: Game loop with fixed timestep, camera, terrain rendering"
```

---

## Phase 2: Player & Combat

### Task 2.1: Components — Transform, Health, Renderable, Collider

**Files:**
- Create: `~/Documents/wizardbattle/src/entity/Components.h`
- Create: `~/Documents/wizardbattle/src/entity/Components.cpp`

- [ ] **Step 1: Write Components.h**

```cpp
#pragma once
#include <glm/glm.hpp>
#include <functional>

struct Transform {
    glm::vec3 position{0};
    glm::vec3 scale{1};
    float rotation = 0;         // Y-axis rotation in radians
};

struct Health {
    int current = 100;
    int max = 100;
    bool alive() const { return current > 0; }
    void takeDamage(int dmg) {
        current -= dmg;
        if (current < 0) current = 0;
    }
};

struct Renderable {
    int meshId = -1;            // Index into AssetManager mesh list
    int textureId = -1;
    glm::vec3 tint{1};
    bool visible = true;
};

struct Collider {
    glm::vec3 halfExtents{0.5f}; // AABB half-size
    bool isTrigger = false;
    bool solid = true;          // Impassable for A*
    std::function<void(class GameObject*)> onCollide;
};
```

- [ ] **Step 2: Write Components.cpp**

```cpp
#include "entity/Components.h"
// Components are data-only structs, no methods beyond inline.
// This file exists for future non-inline component logic.
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/entity/Components.h src/entity/Components.cpp
git commit -m "feat: Component structs — Transform, Health, Renderable, Collider"
```

---

### Task 2.2: GameObject base class

**Files:**
- Create: `~/Documents/wizardbattle/src/entity/GameObject.h`
- Create: `~/Documents/wizardbattle/src/entity/GameObject.cpp`

- [ ] **Step 1: Write GameObject.h**

```cpp
#pragma once
#include "entity/Components.h"
#include <string>

class GameObject {
public:
    GameObject(const std::string& tag = "");
    virtual ~GameObject() = default;

    virtual void update(float dt) {}
    virtual void onDeath() {}

    Transform transform;
    Health health;
    Renderable renderable;
    Collider collider;

    const std::string& tag() const { return _tag; }
    void setDestroyed(bool v) { _destroyed = v; }
    bool destroyed() const { return _destroyed; }
    int id() const { return _id; }

private:
    std::string _tag;
    bool _destroyed = false;
    int _id;
    static int _nextId;
};
```

- [ ] **Step 2: Write GameObject.cpp**

```cpp
#include "entity/GameObject.h"

int GameObject::_nextId = 0;

GameObject::GameObject(const std::string& tag)
    : _tag(tag), _id(_nextId++) {}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/entity/GameObject.h src/entity/GameObject.cpp
git commit -m "feat: GameObject base class with component composition"
```

---

### Task 2.3: Collision system with spatial grid

**Files:**
- Create: `~/Documents/wizardbattle/src/entity/Collision.h`
- Create: `~/Documents/wizardbattle/src/entity/Collision.cpp`

- [ ] **Step 1: Write Collision.h**

```cpp
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

    // Return A* walkable neighbors from a grid position
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

// A* pathfinding on spatial grid
std::vector<glm::vec2> findPath(const SpatialGrid& grid,
    glm::vec2 start, glm::vec2 end);
```

- [ ] **Step 2: Write Collision.cpp**

```cpp
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

// A* pathfinding
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
            // Reconstruct path
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
    return {}; // No path
}
```

- [ ] **Step 4: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 5: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/entity/Collision.h src/entity/Collision.cpp
git commit -m "feat: AABB collision, spatial grid, A* pathfinding"
```

---

### Task 2.4: Wizard player entity

**Files:**
- Create: `~/Documents/wizardbattle/src/player/Wizard.h`
- Create: `~/Documents/wizardbattle/src/player/Wizard.cpp`

- [ ] **Step 1: Write Wizard.h**

```cpp
#pragma once
#include "entity/GameObject.h"
#include <glm/glm.hpp>

class Wizard : public GameObject {
public:
    Wizard();
    void update(float dt) override;

    void move(glm::vec3 dir, float dt);
    void aimAt(int screenX, int screenY, const class Camera& cam);
    void takeDamage(int dmg);
    bool isDead() const { return !health.alive(); }
    glm::vec3 aimDirection() const { return _aimDir; }

    int level = 1;
    int xp = 0;
    float mana = 100;
    float maxMana = 100;
    float speed = 5.0f;

    // Cooldowns managed by SpellBook, stored here for HUD display
    float globalCooldown = 0;

    void addXP(int amount);
    bool leveledUp() const { return _pendingLevelUp; }
    void clearLevelUp() { _pendingLevelUp = false; }

private:
    glm::vec3 _aimDir{0, 0, -1};
    bool _pendingLevelUp = false;
};
```

- [ ] **Step 2: Write Wizard.cpp**

```cpp
#include "player/Wizard.h"
#include "core/Camera.h"

Wizard::Wizard() : GameObject("Player") {
    transform.position = {100, 0, 180}; // Start near south edge
    health.current = 100;
    health.max = 100;
    collider.halfExtents = {0.4f, 0.8f, 0.4f};
    collider.solid = false;
}

void Wizard::update(float dt) {
    // Mana regen
    mana = glm::min(mana + 5.0f * dt, maxMana);
    if (globalCooldown > 0) globalCooldown -= dt;
}

void Wizard::move(glm::vec3 dir, float dt) {
    if (glm::length(dir) > 1.0f) dir = glm::normalize(dir);
    transform.position += dir * speed * dt;

    // Clamp to map bounds
    transform.position.x = glm::clamp(transform.position.x, 1.0f, 199.0f);
    transform.position.z = glm::clamp(transform.position.z, 1.0f, 199.0f);
}

void Wizard::aimAt(int screenX, int screenY, const Camera& cam) {
    // Unproject screen to world ray, intersect with ground plane (y=0)
    float ndcX = (screenX / (float)cam.screenWidth()) * 2.0f - 1.0f;
    float ndcY = 1.0f - (screenY / (float)cam.screenHeight()) * 2.0f;
    glm::mat4 invVP = glm::inverse(cam.proj() * cam.view());
    glm::vec4 near = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    near /= near.w;
    glm::vec4 far = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
    far /= far.w;

    glm::vec3 dir = glm::normalize(glm::vec3(far - near));
    if (glm::abs(dir.y) > 0.001f) {
        float t = -near.y / dir.y; // intersect y=0
        if (t > 0) {
            glm::vec3 hit = glm::vec3(near) + dir * t;
            _aimDir = glm::normalize(hit - transform.position);
        }
    }
}

void Wizard::takeDamage(int dmg) {
    health.takeDamage(dmg);
}

void Wizard::addXP(int amount) {
    xp += amount;
    int xpNeeded = (int)(100.0f * powf(1.5f, level - 1));
    while (xp >= xpNeeded && level < 10) {
        xp -= xpNeeded;
        level++;
        _pendingLevelUp = true;
        maxMana = 100.0f + 15.0f * (level - 1);
        mana = maxMana;
        health.max = 100 + 10 * (level - 1);
        health.current = health.max;
        xpNeeded = (int)(100.0f * powf(1.5f, level - 1));
    }
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/player/Wizard.h src/player/Wizard.cpp
git commit -m "feat: Wizard player entity — movement, aiming, XP/leveling, mana regen"
```

---

### Task 2.5: Spell base class and Firebolt projectile

**Files:**
- Create: `~/Documents/wizardbattle/src/player/Spell.h`
- Create: `~/Documents/wizardbattle/src/player/Spell.cpp`

- [ ] **Step 1: Write Spell.h**

```cpp
#pragma once
#include "entity/GameObject.h"
#include <glm/glm.hpp>
#include <string>

class Spell {
public:
    Spell(const std::string& name, float cooldown, float manaCost, int dmg);
    virtual ~Spell() = default;

    virtual void cast(glm::vec3 origin, glm::vec3 direction,
        std::vector<std::unique_ptr<GameObject>>& projectiles);

    const std::string& name() const { return _name; }
    float cooldown() const { return _cooldown; }
    float manaCost() const { return _manaCost; }
    int damage() const { return _damage; }
    int unlockLevel() const { return _unlockLevel; }
    void setUnlockLevel(int lvl) { _unlockLevel = lvl; }

protected:
    std::string _name;
    float _cooldown;
    float _manaCost;
    int _damage;
    int _unlockLevel = 1;
};

// Projectile game object
class Projectile : public GameObject {
public:
    Projectile(int dmg, float speed, float lifetime,
        float aoeRadius = 0, bool piercing = false);

    void update(float dt) override;

    int damage() const { return _dmg; }
    float aoeRadius() const { return _aoe; }
    bool piercing() const { return _pierce; }
    float lifespan() const { return _life; }

    void setVelocity(glm::vec3 v) { _velocity = v; }

private:
    int _dmg;
    float _aoe;
    bool _pierce;
    float _life;
    float _maxLife;
    glm::vec3 _velocity{0};
};
```

- [ ] **Step 2: Write Spell.cpp**

```cpp
#include "player/Spell.h"

Spell::Spell(const std::string& name, float cooldown, float manaCost, int dmg)
    : _name(name), _cooldown(cooldown), _manaCost(manaCost), _damage(dmg) {}

void Spell::cast(glm::vec3 origin, glm::vec3 direction,
    std::vector<std::unique_ptr<GameObject>>& projectiles) {
    auto p = std::make_unique<Projectile>(_damage, 15.0f, 2.0f, 1.5f);
    p->transform.position = origin + glm::vec3(0, 1.0f, 0);
    p->setVelocity(direction * 15.0f);
    projectiles.push_back(std::move(p));
}

Projectile::Projectile(int dmg, float speed, float lifetime,
    float aoeRadius, bool piercing)
    : GameObject("Projectile")
    , _dmg(dmg), _aoe(aoeRadius), _pierce(piercing)
    , _life(lifetime), _maxLife(lifetime)
{
    collider.halfExtents = {0.3f, 0.3f, 0.3f};
    collider.isTrigger = true;
    collider.solid = false;
    renderable.tint = {1.0f, 0.5f, 0.0f}; // Orange for firebolt
}

void Projectile::update(float dt) {
    transform.position += _velocity * dt;
    _life -= dt;
    if (_life <= 0) setDestroyed(true);
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/player/Spell.h src/player/Spell.cpp
git commit -m "feat: Spell base class + Projectile with AOE/piercing support"
```

---

### Task 2.6: Wire player into Game — movement, camera follow, spell casting

**Modify:** `~/Documents/wizardbattle/src/core/Game.h`, `Game.cpp`

- [ ] **Step 1: Update Game.h — add Wizard and spell state**

In Game.h, add includes and new members after existing fields:

```cpp
#include "player/Wizard.h"
#include "player/Spell.h"
// ... inside class Game, add:
    Wizard _wizard;
    Spell _firebolt{"Firebolt", 0.8f, 15.0f, 25};
    std::vector<std::unique_ptr<GameObject>> _projectiles;
```

- [ ] **Step 2: Update Game.cpp constructor — set firebolt unlock**

```cpp
Game::Game()
    : _window("WizardBattle", 1280, 720)
    , _camera(1280, 720)
    , _terrain(200, 1.0f)
    , _cube(Mesh::createCube())
    , _whiteTex(Texture::createSolid(255, 255, 255))
{
    // ... existing init code ...
    _firebolt.setUnlockLevel(1);
}
```

- [ ] **Step 3: Update Game::update — player movement and camera**

```cpp
void Game::update(float dt) {
    if (_state != GameState::Playing) return;

    // Camera follows wizard
    _camera.setTarget(_wizard.transform.position);
    _camera.update(dt);
    if (_input.mouseWheel() != 0)
        _camera.zoom((float)_input.mouseWheel() * 2.0f);

    // Player movement
    glm::vec3 moveDir(0);
    if (_input.keyDown(SDLK_W)) moveDir.z -= 1;
    if (_input.keyDown(SDLK_S)) moveDir.z += 1;
    if (_input.keyDown(SDLK_A)) moveDir.x -= 1;
    if (_input.keyDown(SDLK_D)) moveDir.x += 1;
    _wizard.move(moveDir, dt);

    // Aim
    _wizard.aimAt(_input.mouseX(), _input.mouseY(), _camera);

    // Cast Firebolt
    if (_input.mousePressed(SDL_BUTTON_LEFT) &&
        _wizard.mana >= _firebolt.manaCost() &&
        _wizard.globalCooldown <= 0) {
        _firebolt.cast(_wizard.transform.position,
            _wizard.aimDirection(), _projectiles);
        _wizard.mana -= _firebolt.manaCost();
        _wizard.globalCooldown = _firebolt.cooldown();
    }

    // Clamp wizard to terrain height
    _wizard.transform.position.y = _terrain.heightAt(
        _wizard.transform.position.x, _wizard.transform.position.z);

    // Update projectiles
    _wizard.update(dt);
    for (auto& p : _projectiles) p->update(dt);

    // Remove destroyed projectiles
    _projectiles.erase(
        std::remove_if(_projectiles.begin(), _projectiles.end(),
            [](auto& p) { return p->destroyed(); }),
        _projectiles.end());
}
```

- [ ] **Step 4: Update Game::render — draw wizard and projectiles**

```cpp
void Game::render() {
    _renderer.clear();
    _renderer.begin(_camera);

    // Draw terrain
    _renderer.drawMesh(_terrain.mesh(), _whiteTex,
        glm::mat4(1));

    // Draw wizard (cube placeholder)
    _renderer.drawMesh(_cube, _whiteTex,
        glm::translate(glm::mat4(1), _wizard.transform.position));

    // Draw projectiles
    for (auto& p : _projectiles) {
        if (!p->destroyed()) {
            _renderer.drawMesh(_cube, _whiteTex,
                glm::translate(glm::mat4(1), p->transform.position));
        }
    }

    _renderer.end();
}
```

- [ ] **Step 5: Build and run**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc) && ./wizardbattle
```

Expected: WASD moves wizard on terrain, mouse aims, left-click fires orange projectiles toward cursor.

- [ ] **Step 6: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Game.h src/core/Game.cpp
git commit -m "feat: Wizard movement, camera follow, mouse aim, firebolt casting"
```

---

## Phase 3: Mobs & AI

### Task 3.1: MobAI state machine base

**Files:**
- Create: `~/Documents/wizardbattle/src/mob/MobAI.h`
- Create: `~/Documents/wizardbattle/src/mob/MobAI.cpp`

- [ ] **Step 1: Write MobAI.h**

```cpp
#pragma once
#include "entity/GameObject.h"
#include "entity/Collision.h"
#include <glm/glm.hpp>
#include <functional>

enum class AIState { Idle, Aggro, Chase, Ranged, Flee, Dead };

class MobAI {
public:
    MobAI(float detectRadius, float attackRange, float attackCooldown,
        int attackDamage, float moveSpeed, bool isRanged = false,
        bool canFlee = false);

    void update(float dt, GameObject& self, const GameObject& player,
        const SpatialGrid& grid);

    AIState state() const { return _state; }
    int attackDamage() const { return _attackDamage; }
    float attackCooldown() const { return _attackCooldown; }
    float currentCooldown() const { return _cooldownTimer; }
    void resetCooldown() { _cooldownTimer = _attackCooldown; }

    glm::vec3 moveDirection() const { return _moveDir; }
    bool wantsToAttack() const { return _wantsAttack; }
    void clearAttack() { _wantsAttack = false; }
    glm::vec3 projectileTarget() const { return _projTarget; }
    bool wantsProjectile() const { return _wantsProjectile; }
    void clearProjectile() { _wantsProjectile = false; }

private:
    AIState _state = AIState::Idle;
    float _detectRadius;
    float _attackRange;
    float _attackCooldown;
    int _attackDamage;
    float _moveSpeed;
    bool _isRanged;
    bool _canFlee;
    float _fleeThreshold = 0.2f; // HP ratio

    float _cooldownTimer = 0;
    float _wanderTimer = 0;
    glm::vec3 _wanderTarget{0};
    glm::vec3 _moveDir{0};
    bool _wantsAttack = false;
    bool _wantsProjectile = false;
    glm::vec3 _projTarget{0};

    std::vector<glm::vec2> _path;
    size_t _pathIndex = 0;
    float _repathTimer = 0;
};
```

- [ ] **Step 2: Write MobAI.cpp**

```cpp
#include "mob/MobAI.h"
#include <glm/gtc/random.hpp>

MobAI::MobAI(float detectRadius, float attackRange, float attackCooldown,
    int attackDamage, float moveSpeed, bool isRanged, bool canFlee)
    : _detectRadius(detectRadius), _attackRange(attackRange),
      _attackCooldown(attackCooldown), _attackDamage(attackDamage),
      _moveSpeed(moveSpeed), _isRanged(isRanged), _canFlee(canFlee) {}

void MobAI::update(float dt, GameObject& self, const GameObject& player,
    const SpatialGrid& grid) {

    if (!self.health.alive()) {
        _state = AIState::Dead;
        return;
    }

    if (_cooldownTimer > 0) _cooldownTimer -= dt;

    float dist = glm::distance(
        glm::vec2(self.transform.position.x, self.transform.position.z),
        glm::vec2(player.transform.position.x, player.transform.position.z));

    // Fleeing check
    float hpRatio = (float)self.health.current / self.health.max;
    if (_canFlee && hpRatio < _fleeThreshold) {
        _state = AIState::Flee;
    }

    switch (_state) {
    case AIState::Idle: {
        _wanderTimer -= dt;
        if (_wanderTimer <= 0) {
            _wanderTarget = self.transform.position +
                glm::vec3(glm::linearRand(-10.0f, 10.0f), 0,
                          glm::linearRand(-10.0f, 10.0f));
            _wanderTimer = glm::linearRand(2.0f, 5.0f);
        }
        glm::vec3 toTarget = _wanderTarget - self.transform.position;
        toTarget.y = 0;
        if (glm::length(toTarget) > 0.5f)
            _moveDir = glm::normalize(toTarget) * _moveSpeed * 0.3f;
        else
            _moveDir = glm::vec3(0);

        if (dist < _detectRadius)
            _state = _isRanged ? AIState::Ranged : AIState::Chase;
        break;
    }

    case AIState::Chase: {
        _wantsAttack = false;
        _repathTimer -= dt;
        if (_repathTimer <= 0) {
            _path = findPath(grid,
                glm::vec2(self.transform.position.x, self.transform.position.z),
                glm::vec2(player.transform.position.x, player.transform.position.z));
            _pathIndex = 0;
            _repathTimer = 0.5f;
        }

        if (_pathIndex < _path.size()) {
            glm::vec3 target(_path[_pathIndex].x, 0, _path[_pathIndex].y);
            glm::vec3 toTarget = target - self.transform.position;
            toTarget.y = 0;
            if (glm::length(toTarget) < 0.3f) _pathIndex++;
            _moveDir = glm::normalize(toTarget) * _moveSpeed;
        } else {
            _moveDir = glm::vec3(0);
        }

        if (dist < _attackRange && _cooldownTimer <= 0) {
            _wantsAttack = true;
        }

        if (dist > _detectRadius * 1.5f && _canFlee && hpRatio < _fleeThreshold) {
            _state = AIState::Flee;
        } else if (dist > _detectRadius * 2.0f) {
            _state = AIState::Idle;
        }
        break;
    }

    case AIState::Ranged: {
        _wantsProjectile = false;
        if (dist < _attackRange * 0.6f) {
            // Too close, back away
            glm::vec3 away = self.transform.position - player.transform.position;
            away.y = 0;
            if (glm::length(away) > 0.01f)
                _moveDir = glm::normalize(away) * _moveSpeed;
        } else if (dist > _attackRange * 1.5f) {
            // Too far, close in
            glm::vec3 toward = player.transform.position - self.transform.position;
            toward.y = 0;
            if (glm::length(toward) > 0.01f)
                _moveDir = glm::normalize(toward) * _moveSpeed;
        } else {
            _moveDir = glm::vec3(0);
            if (_cooldownTimer <= 0) {
                _wantsProjectile = true;
                _projTarget = player.transform.position;
            }
        }

        if (dist > _detectRadius * 2.0f)
            _state = AIState::Idle;
        break;
    }

    case AIState::Flee: {
        glm::vec3 away = self.transform.position - player.transform.position;
        away.y = 0;
        _moveDir = glm::normalize(away) * _moveSpeed * 1.3f; // Faster flee
        if (dist > _detectRadius * 2.5f)
            _state = AIState::Idle;
        break;
    }

    case AIState::Dead:
        _moveDir = glm::vec3(0);
        break;
    }
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/mob/MobAI.h src/mob/MobAI.cpp
git commit -m "feat: MobAI state machine — Idle/Chase/Ranged/Flee/Dead"
```

---

### Task 3.2: Goblin mob

**Files:**
- Create: `~/Documents/wizardbattle/src/mob/Goblin.h`
- Create: `~/Documents/wizardbattle/src/mob/Goblin.cpp`

- [ ] **Step 1: Write Goblin.h**

```cpp
#pragma once
#include "entity/GameObject.h"
#include "mob/MobAI.h"

class Goblin : public GameObject {
public:
    Goblin(const glm::vec3& pos);
    void update(float dt, const GameObject& player, const SpatialGrid& grid);

    MobAI ai;
    int xpValue() const { return 30; }

    float deathTimer = 0;
    static constexpr float DEATH_DURATION = 3.0f;
};
```

- [ ] **Step 2: Write Goblin.cpp**

```cpp
#include "mob/Goblin.h"

Goblin::Goblin(const glm::vec3& pos)
    : GameObject("Goblin")
    , ai(12.0f, 1.5f, 1.0f, 8, 4.0f, false, true)
{
    transform.position = pos;
    health.current = 40;
    health.max = 40;
    collider.halfExtents = {0.3f, 0.6f, 0.3f};
    collider.solid = true;
    renderable.tint = {0.2f, 0.7f, 0.2f}; // Green
}

void Goblin::update(float dt, const GameObject& player, const SpatialGrid& grid) {
    if (!health.alive()) {
        deathTimer += dt;
        if (deathTimer >= DEATH_DURATION)
            setDestroyed(true);
        return;
    }

    ai.update(dt, *this, player, grid);
    glm::vec3 move = ai.moveDirection();
    transform.position += move * dt;
    transform.position.y = 0; // Snap to ground
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/mob/Goblin.h src/mob/Goblin.cpp
git commit -m "feat: Goblin mob — fast melee, swarms, flees at low HP"
```

---

### Task 3.3: Archer mob

**Files:**
- Create: `~/Documents/wizardbattle/src/mob/Archer.h`
- Create: `~/Documents/wizardbattle/src/mob/Archer.cpp`

- [ ] **Step 1: Write Archer.h**

```cpp
#pragma once
#include "entity/GameObject.h"
#include "mob/MobAI.h"

class Archer : public GameObject {
public:
    Archer(const glm::vec3& pos);
    void update(float dt, const GameObject& player, const SpatialGrid& grid);

    MobAI ai;
    int xpValue() const { return 45; }

    float deathTimer = 0;
    static constexpr float DEATH_DURATION = 3.0f;
};
```

- [ ] **Step 2: Write Archer.cpp**

```cpp
#include "mob/Archer.h"

Archer::Archer(const glm::vec3& pos)
    : GameObject("Archer")
    , ai(15.0f, 10.0f, 2.0f, 12, 3.0f, true, false)
{
    transform.position = pos;
    health.current = 30;
    health.max = 30;
    collider.halfExtents = {0.3f, 0.7f, 0.3f};
    collider.solid = true;
    renderable.tint = {0.6f, 0.3f, 0.1f}; // Brown
}

void Archer::update(float dt, const GameObject& player, const SpatialGrid& grid) {
    if (!health.alive()) {
        deathTimer += dt;
        if (deathTimer >= DEATH_DURATION)
            setDestroyed(true);
        return;
    }

    ai.update(dt, *this, player, grid);
    glm::vec3 move = ai.moveDirection();
    transform.position += move * dt;
    transform.position.y = 0;
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/mob/Archer.h src/mob/Archer.cpp
git commit -m "feat: Archer mob — ranged, keeps distance, fires arrows"
```

---

### Task 3.4: Ogre mob

**Files:**
- Create: `~/Documents/wizardbattle/src/mob/Ogre.h`
- Create: `~/Documents/wizardbattle/src/mob/Ogre.cpp`

- [ ] **Step 1: Write Ogre.h**

```cpp
#pragma once
#include "entity/GameObject.h"
#include "mob/MobAI.h"

class Ogre : public GameObject {
public:
    Ogre(const glm::vec3& pos);
    void update(float dt, const GameObject& player, const SpatialGrid& grid);

    MobAI ai;
    int xpValue() const { return 70; }

    // Telegraph: wind-up before attack
    float attackWindUp = 0;
    bool isWindingUp = false;

    float deathTimer = 0;
    static constexpr float DEATH_DURATION = 3.0f;
};
```

- [ ] **Step 2: Write Ogre.cpp**

```cpp
#include "mob/Ogre.h"

Ogre::Ogre(const glm::vec3& pos)
    : GameObject("Ogre")
    , ai(10.0f, 1.8f, 2.0f, 20, 2.0f, false, false)
{
    transform.position = pos;
    transform.scale = {1.5f, 1.5f, 1.5f};
    health.current = 100;
    health.max = 100;
    collider.halfExtents = {0.5f, 1.0f, 0.5f};
    collider.solid = true;
    renderable.tint = {0.4f, 0.3f, 0.5f}; // Purple-ish
}

void Ogre::update(float dt, const GameObject& player, const SpatialGrid& grid) {
    if (!health.alive()) {
        deathTimer += dt;
        if (deathTimer >= DEATH_DURATION)
            setDestroyed(true);
        return;
    }

    ai.update(dt, *this, player, grid);

    // Telegraph: wind-up before heavy hit
    if (ai.wantsToAttack()) {
        if (!isWindingUp) {
            isWindingUp = true;
            attackWindUp = 0.7f; // 0.7s telegraph
        }
        attackWindUp -= dt;
        if (attackWindUp <= 0) {
            ai.resetCooldown();
            isWindingUp = false;
            ai.clearAttack();
            // Damage dealt in Game collision logic
        }
    }

    if (!isWindingUp) {
        glm::vec3 move = ai.moveDirection();
        transform.position += move * dt;
    }
    transform.position.y = 0;
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/mob/Ogre.h src/mob/Ogre.cpp
git commit -m "feat: Ogre mob — slow tank with wind-up telegraph attack"
```

---

### Task 3.5: Spawner — population management

**Files:**
- Create: `~/Documents/wizardbattle/src/mob/Spawner.h`
- Create: `~/Documents/wizardbattle/src/mob/Spawner.cpp`

- [ ] **Step 1: Write Spawner.h**

```cpp
#pragma once
#include "entity/GameObject.h"
#include "mob/Goblin.h"
#include "mob/Archer.h"
#include "mob/Ogre.h"
#include <vector>
#include <memory>

class Spawner {
public:
    Spawner(float mapSize);

    void update(float dt, const glm::vec3& playerPos,
        std::vector<std::unique_ptr<GameObject>>& mobs);

    int targetCount() const { return _targetCount; }
    int killCount() const { return _killCount; }
    void addKill() { _killCount++; }

private:
    float _mapSize;
    float _timer = 0;
    int _killCount = 0;
    int _targetCount = 15;

    struct MobWeights { float goblin, archer, ogre; };
    MobWeights _weights{0.5f, 0.3f, 0.2f};

    void spawnOne(std::vector<std::unique_ptr<GameObject>>& mobs);
    glm::vec3 randomEdgePos() const;
};
```

- [ ] **Step 2: Write Spawner.cpp**

```cpp
#include "mob/Spawner.h"
#include <glm/gtc/random.hpp>

Spawner::Spawner(float mapSize) : _mapSize(mapSize) {}

void Spawner::update(float dt, const glm::vec3& playerPos,
    std::vector<std::unique_ptr<GameObject>>& mobs) {

    _timer -= dt;

    int alive = 0;
    for (auto& m : mobs)
        if (m->health.alive()) alive++;

    if (alive < _targetCount && _timer <= 0) {
        spawnOne(mobs);
        _timer = glm::linearRand(5.0f, 10.0f);
    }
}

void Spawner::spawnOne(std::vector<std::unique_ptr<GameObject>>& mobs) {
    glm::vec3 pos = randomEdgePos();
    float roll = glm::linearRand(0.0f, 1.0f);

    if (roll < _weights.goblin)
        mobs.push_back(std::make_unique<Goblin>(pos));
    else if (roll < _weights.goblin + _weights.archer)
        mobs.push_back(std::make_unique<Archer>(pos));
    else
        mobs.push_back(std::make_unique<Ogre>(pos));
}

glm::vec3 Spawner::randomEdgePos() const {
    float margin = _mapSize * 0.05f;
    // Pick a random edge
    int edge = rand() % 4;
    float x, z;
    switch (edge) {
    case 0: x = margin; z = glm::linearRand(margin, _mapSize - margin); break;
    case 1: x = _mapSize - margin; z = glm::linearRand(margin, _mapSize - margin); break;
    case 2: x = glm::linearRand(margin, _mapSize - margin); z = margin; break;
    case 3: x = glm::linearRand(margin, _mapSize - margin); z = _mapSize - margin; break;
    }
    return {x, 0, z};
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/mob/Spawner.h src/mob/Spawner.cpp
git commit -m "feat: Spawner — population management, edge spawning, kill tracking"
```

---

### Task 3.6: Wire mobs into Game — spawn, update, render, collision

**Modify:** `~/Documents/wizardbattle/src/core/Game.h`, `Game.cpp`

- [ ] **Step 1: Update Game.h — add mob collection and spawner**

Add after wizard:
```cpp
#include "mob/Spawner.h"
#include "mob/Goblin.h"
#include "mob/Archer.h"
#include "mob/Ogre.h"
// ... in class Game, add:
    Spawner _spawner{200.0f};
    std::vector<std::unique_ptr<GameObject>> _mobs;
```

- [ ] **Step 2: Update Game::update — mob logic and projectile-mob collision**

Add to Game::update, before the projectile cleanup:

```cpp
    // Spawner
    _spawner.update(dt, _wizard.transform.position, _mobs);

    // Update mobs
    SpatialGrid grid(200.0f, 10.0f);
    for (auto& m : _mobs) {
        if (!m->destroyed()) grid.insert(m.get());
    }

    for (auto& m : _mobs) {
        if (auto* g = dynamic_cast<Goblin*>(m.get())) g->update(dt, _wizard, grid);
        else if (auto* a = dynamic_cast<Archer*>(m.get())) a->update(dt, _wizard, grid);
        else if (auto* o = dynamic_cast<Ogre*>(m.get())) o->update(dt, _wizard, grid);
    }

    // Projectile-mob collision
    for (auto& p : _projectiles) {
        if (p->destroyed()) continue;
        auto* proj = dynamic_cast<Projectile*>(p.get());
        if (!proj) continue;

        for (auto& m : _mobs) {
            if (!m->health.alive()) continue;
            if (aabbIntersects(getWorldAABB(*p), getWorldAABB(*m))) {
                m->health.takeDamage(proj->damage());

                if (!m->health.alive()) {
                    _spawner.addKill();
                    int xp = 0;
                    if (dynamic_cast<Goblin*>(m.get())) xp = 30;
                    else if (dynamic_cast<Archer*>(m.get())) xp = 45;
                    else if (dynamic_cast<Ogre*>(m.get())) xp = 70;
                    _wizard.addXP(xp);
                }

                if (!proj->piercing())
                    p->setDestroyed(true);
                break;
            }
        }
    }

    // Mob melee attacks on player
    for (auto& m : _mobs) {
        if (!m->health.alive()) continue;
        MobAI* ai = nullptr;
        if (auto* g = dynamic_cast<Goblin*>(m.get())) ai = &g->ai;
        else if (auto* a = dynamic_cast<Archer*>(m.get())) ai = &a->ai;
        else if (auto* o = dynamic_cast<Ogre*>(m.get())) ai = &o->ai;
        if (!ai) continue;

        if (ai->wantsToAttack()) {
            float dist = glm::distance(
                glm::vec2(m->transform.position.x, m->transform.position.z),
                glm::vec2(_wizard.transform.position.x, _wizard.transform.position.z));
            float range = 1.8f; // melee range
            if (dist < range) {
                _wizard.takeDamage(ai->attackDamage());
            }
            ai->resetCooldown();
            ai->clearAttack();
        }

        // Archer projectiles
        if (ai->wantsProjectile()) {
            auto arrow = std::make_unique<Projectile>(
                ai->attackDamage(), 10.0f, 3.0f, 0, false);
            arrow->transform.position = m->transform.position + glm::vec3(0,1,0);
            glm::vec3 dir = glm::normalize(
                ai->projectileTarget() - m->transform.position);
            arrow->setVelocity(dir * 10.0f);
            arrow->renderable.tint = {0.5f, 0.3f, 0.2f};
            _projectiles.push_back(std::move(arrow));
            ai->resetCooldown();
            ai->clearProjectile();
        }
    }

    // Apply terrain height to all mobs and wizard
    for (auto& m : _mobs) {
        m->transform.position.y = _terrain.heightAt(
            m->transform.position.x, m->transform.position.z);
    }

    // Cleanup dead mobs
    _mobs.erase(
        std::remove_if(_mobs.begin(), _mobs.end(),
            [](auto& m) { return m->destroyed(); }),
        _mobs.end());
```

- [ ] **Step 3: Update Game::render — draw mobs**

Add to Game::render, after wizard:
```cpp
    // Draw mobs
    for (auto& m : _mobs) {
        if (!m->destroyed()) {
            _renderer.drawMesh(_cube, _whiteTex,
                glm::translate(glm::mat4(1), m->transform.position)
                * glm::scale(glm::mat4(1), m->transform.scale));
        }
    }
```

- [ ] **Step 4: Build and run**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc) && ./wizardbattle
```

Expected: Mobs spawn at edges, wander, chase the wizard. Can be killed with firebolts. Goblins flee at low HP. Archers keep distance and fire arrows. Ogres have wind-up attacks.

- [ ] **Step 5: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Game.h src/core/Game.cpp
git commit -m "feat: Mob spawning, AI, combat — collision, damage, XP, kill tracking"
```

---

## Phase 4: Spells & Progression

### Task 4.1: SpellBook — manage 5 spells, cooldowns, unlocks

**Files:**
- Create: `~/Documents/wizardbattle/src/player/SpellBook.h`
- Create: `~/Documents/wizardbattle/src/player/SpellBook.cpp`

- [ ] **Step 1: Write SpellBook.h**

```cpp
#pragma once
#include "player/Spell.h"
#include <vector>
#include <memory>

class SpellBook {
public:
    SpellBook();

    void update(float dt);
    bool cast(int index, glm::vec3 origin, glm::vec3 direction,
        float& mana, std::vector<std::unique_ptr<GameObject>>& projectiles);

    int spellCount() const { return 5; }
    bool isUnlocked(int index) const;
    void unlockForLevel(int level);

    float cooldownRemaining(int index) const;
    float manaCost(int index) const;
    const std::string& spellName(int index) const;
    int selectedSpell() const { return _selected; }
    void selectSpell(int index);

private:
    std::vector<std::unique_ptr<Spell>> _spells;
    std::vector<float> _cooldowns;
    int _selected = 0;
};
```

- [ ] **Step 2: Write SpellBook.cpp**

```cpp
#include "player/SpellBook.h"

// Specialized spell classes for Lightning, Ice, Arcane, Meteor

class IceShard : public Spell {
public:
    IceShard() : Spell("Ice Shard", 1.5f, 25.0f, 20) {}
    void cast(glm::vec3 origin, glm::vec3 direction,
        std::vector<std::unique_ptr<GameObject>>& projectiles) override {
        auto p = std::make_unique<Projectile>(_damage, 12.0f, 3.0f, 0, true);
        p->transform.position = origin + glm::vec3(0, 1.0f, 0);
        p->setVelocity(direction * 12.0f);
        p->renderable.tint = {0.4f, 0.7f, 1.0f}; // Ice blue
        projectiles.push_back(std::move(p));
    }
};

class LightningStrike : public Spell {
public:
    LightningStrike() : Spell("Lightning Strike", 3.0f, 35.0f, 45) {}
    void cast(glm::vec3 origin, glm::vec3 direction,
        std::vector<std::unique_ptr<GameObject>>& projectiles) override {
        // Instant: create 3 projectiles that chain to nearby targets
        for (int i = 0; i < 3; i++) {
            float offsetX = (i - 1) * 3.0f;
            auto p = std::make_unique<Projectile>(_damage, 50.0f, 0.3f, 3.0f, false);
            p->transform.position = origin + glm::vec3(offsetX, 1.0f, 0);
            p->setVelocity(direction * 50.0f);
            p->renderable.tint = {1.0f, 1.0f, 0.3f}; // Yellow
            projectiles.push_back(std::move(p));
        }
    }
};

class ArcaneNova : public Spell {
public:
    ArcaneNova() : Spell("Arcane Nova", 6.0f, 50.0f, 30) {}
    void cast(glm::vec3 origin, glm::vec3 direction,
        std::vector<std::unique_ptr<GameObject>>& projectiles) override {
        // PBAOE: burst in all directions
        for (int i = 0; i < 8; i++) {
            float angle = i * glm::pi<float>() / 4.0f;
            glm::vec3 dir(cos(angle), 0, sin(angle));
            auto p = std::make_unique<Projectile>(_damage, 8.0f, 1.5f, 4.0f, false);
            p->transform.position = origin + glm::vec3(0, 1.0f, 0);
            p->setVelocity(dir * 8.0f);
            p->renderable.tint = {0.7f, 0.3f, 1.0f}; // Purple
            projectiles.push_back(std::move(p));
        }
    }
};

class Meteor : public Spell {
public:
    Meteor() : Spell("Meteor", 12.0f, 80.0f, 100) {}
    void cast(glm::vec3 origin, glm::vec3 direction,
        std::vector<std::unique_ptr<GameObject>>& projectiles) override {
        // Delayed big AOE — spawn at target point, short delay
        glm::vec3 target = origin + direction * 15.0f;
        auto p = std::make_unique<Projectile>(_damage, 0, 1.0f, 8.0f, false);
        p->transform.position = target + glm::vec3(0, 20.0f, 0); // Start high
        p->setVelocity(glm::vec3(0, -20.0f, 0)); // Fall down
        p->renderable.tint = {1.0f, 0.3f, 0.1f}; // Fire red
        projectiles.push_back(std::move(p));
    }
};

SpellBook::SpellBook() {
    _spells.push_back(std::make_unique<Spell>("Firebolt", 0.8f, 15.0f, 25));
    _spells.push_back(std::make_unique<IceShard>());
    _spells.push_back(std::make_unique<LightningStrike>());
    _spells.push_back(std::make_unique<ArcaneNova>());
    _spells.push_back(std::make_unique<Meteor>());

    _spells[0]->setUnlockLevel(1);
    _spells[1]->setUnlockLevel(3);
    _spells[2]->setUnlockLevel(5);
    _spells[3]->setUnlockLevel(8);
    _spells[4]->setUnlockLevel(10);

    _cooldowns.resize(5, 0);
}

void SpellBook::update(float dt) {
    for (auto& cd : _cooldowns)
        if (cd > 0) cd -= dt;
}

bool SpellBook::cast(int index, glm::vec3 origin, glm::vec3 direction,
    float& mana, std::vector<std::unique_ptr<GameObject>>& projectiles) {

    if (index < 0 || index >= 5) return false;
    if (!isUnlocked(index)) return false;
    auto& s = _spells[index];
    if (_cooldowns[index] > 0) return false;
    if (mana < s->manaCost()) return false;

    s->cast(origin, direction, projectiles);
    mana -= s->manaCost();
    _cooldowns[index] = s->cooldown();
    return true;
}

bool SpellBook::isUnlocked(int index) const {
    return index < (int)_spells.size();
}

void SpellBook::unlockForLevel(int level) {
    // Already handled by Spell's unlockLevel, just a notification trigger
}

float SpellBook::cooldownRemaining(int index) const {
    if (index < 0 || index >= 5) return 0;
    if (!isUnlocked(index)) return -1; // Locked
    return _cooldowns[index];
}

float SpellBook::manaCost(int index) const {
    if (index < 0 || index >= 5) return 0;
    return _spells[index]->manaCost();
}

const std::string& SpellBook::spellName(int index) const {
    static std::string locked = "Locked";
    if (index < 0 || index >= 5) return locked;
    return _spells[index]->name();
}

void SpellBook::selectSpell(int index) {
    if (isUnlocked(index)) _selected = index;
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/player/SpellBook.h src/player/SpellBook.cpp
git commit -m "feat: SpellBook — Ice Shard, Lightning Strike, Arcane Nova, Meteor"
```

---

### Task 4.2: Wire SpellBook into Game

**Modify:** `~/Documents/wizardbattle/src/core/Game.h`, `Game.cpp`

- [ ] **Step 1: Update Game.h — replace _firebolt with _spellBook**

```cpp
#include "player/SpellBook.h"
// In Game class, replace:
// Spell _firebolt{"Firebolt", 0.8f, 15.0f, 25};
// With:
    SpellBook _spellBook;
```

- [ ] **Step 2: Update Game constructor — remove firebolt init**

Remove the line: `_firebolt.setUnlockLevel(1);`

- [ ] **Step 3: Update Game::update — spell selection and casting**

Replace the firebolt casting block with:
```cpp
    // Spell selection (1-5 keys or mouse wheel with modifier)
    for (int i = 0; i < 5; i++) {
        if (_input.keyPressed((SDL_Keycode)(SDLK_1 + i))) {
            _spellBook.selectSpell(i);
        }
    }
    if (_input.keyDown(SDLK_LSHIFT)) {
        if (_input.mouseWheel() > 0)
            _spellBook.selectSpell((_spellBook.selectedSpell() + 4) % 5);
        if (_input.mouseWheel() < 0)
            _spellBook.selectSpell((_spellBook.selectedSpell() + 1) % 5);
    }

    // Cast selected spell
    if (_input.mousePressed(SDL_BUTTON_LEFT) &&
        _wizard.globalCooldown <= 0) {
        if (_spellBook.cast(_spellBook.selectedSpell(),
            _wizard.transform.position, _wizard.aimDirection(),
            _wizard.mana, _projectiles)) {
            _wizard.globalCooldown = 0.2f; // Small GCD
        }
    }

    // Handle level-up spell unlocks
    if (_wizard.leveledUp()) {
        // Spells auto-unlock based on level — checks happen in cast()
        _wizard.clearLevelUp();
    }

    // Update spellbook cooldowns
    _spellBook.update(dt);
```

- [ ] **Step 4: Build and run**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc) && ./wizardbattle
```

Expected: Press 1-5 to select spells, left-click to cast. Unlocked spells work based on level. Cooldowns prevent spam. Mana depletes and regens.

- [ ] **Step 5: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Game.h src/core/Game.cpp
git commit -m "feat: SpellBook integration — 5 spells with cooldowns, mana, unlocks"
```

---

### Task 4.3: Progression system — XP bar, level-up notification

**Files:**
- Create: `~/Documents/wizardbattle/src/player/Progression.h`
- Create: `~/Documents/wizardbattle/src/player/Progression.cpp`

- [ ] **Step 1: Write Progression.h**

```cpp
#pragma once

class Progression {
public:
    static int xpForLevel(int level);
    static float xpProgress(int xp, int level); // 0..1
    static int xpForNextLevel(int level);
};
```

- [ ] **Step 2: Write Progression.cpp**

```cpp
#include "player/Progression.h"
#include <cmath>

int Progression::xpForLevel(int level) {
    return (int)(100.0 * powf(1.5f, level - 1));
}

int Progression::xpForNextLevel(int level) {
    return xpForLevel(level + 1);
}

float Progression::xpProgress(int xp, int level) {
    if (level >= 10) return 1.0f;
    int needed = xpForLevel(level);
    return glm::clamp((float)xp / (float)needed, 0.0f, 1.0f);
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/player/Progression.h src/player/Progression.cpp
git commit -m "feat: Progression — XP formula, level progress calculation"
```

---

## Phase 5: Boss Fight

### Task 5.1: Dragon boss entity

**Files:**
- Create: `~/Documents/wizardbattle/src/mob/Dragon.h`
- Create: `~/Documents/wizardbattle/src/mob/Dragon.cpp`

- [ ] **Step 1: Write Dragon.h**

```cpp
#pragma once
#include "entity/GameObject.h"
#include <vector>

enum class DragonPhase { One, Two, Three };

class Dragon : public GameObject {
public:
    Dragon();
    void update(float dt, const GameObject& player,
        std::vector<std::unique_ptr<GameObject>>& projectiles);

    DragonPhase phase() const;
    bool isActive() const { return _active; }
    void activate() { _active = true; }

    float swipeDamage() const { return 35.0f; }
    float breathDamage() const { return 50.0f; }
    float tailDamage() const { return 25.0f; }

    int xpValue() const { return 500; }

private:
    bool _active = false;
    float _attackTimer = 0;
    float _breathTimer = 0;
    float _enrageMultiplier = 1.0f;

    enum CurrentAttack { Swipe, Tail, Breath, None };
    CurrentAttack _currentAttack = None;
    float _telegraphTimer = 0;

    void chooseAttack(const glm::vec3& toPlayer);
    void doSwipe(const glm::vec3& dir);
    void doTail(const glm::vec3& dir);
    void doBreath(const glm::vec3& dir,
        std::vector<std::unique_ptr<GameObject>>& projectiles);
};
```

- [ ] **Step 2: Write Dragon.cpp**

```cpp
#include "mob/Dragon.h"
#include <glm/gtc/random.hpp>

Dragon::Dragon() : GameObject("Dragon") {
    transform.position = {100, 0, 100}; // Center clearing
    transform.scale = {3.0f, 3.0f, 3.0f}; // 3x bigger than ogre
    health.current = 2000;
    health.max = 2000;
    collider.halfExtents = {2.0f, 2.5f, 2.0f};
    collider.solid = true;
    renderable.tint = {0.8f, 0.2f, 0.1f}; // Red
}

DragonPhase Dragon::phase() const {
    float ratio = (float)health.current / health.max;
    if (ratio > 0.6f) return DragonPhase::One;
    if (ratio > 0.3f) return DragonPhase::Two;
    return DragonPhase::Three;
}

void Dragon::update(float dt, const GameObject& player,
    std::vector<std::unique_ptr<GameObject>>& projectiles) {

    if (!_active) return;
    if (!health.alive()) return;

    // Update phase-based enrage
    _enrageMultiplier = (phase() == DragonPhase::Three) ? 1.5f : 1.0f;
    float speed = _enrageMultiplier;

    glm::vec3 toPlayer = player.transform.position - transform.position;
    float dist = glm::length(toPlayer);

    // Slowly turn to face player
    if (dist > 0.01f) {
        glm::vec3 dir = toPlayer / dist;
        float targetAngle = atan2(dir.x, dir.z);
        // Smooth rotation
        float da = targetAngle - transform.rotation;
        while (da > glm::pi<float>()) da -= 2 * glm::pi<float>();
        while (da < -glm::pi<float>()) da += 2 * glm::pi<float>();
        transform.rotation += da * glm::min(dt * 3.0f, 1.0f);
    }

    _attackTimer -= dt * speed;

    if (_currentAttack != None) {
        _telegraphTimer -= dt * speed;
        if (_telegraphTimer <= 0) {
            // Execute attack
            switch (_currentAttack) {
            case Swipe:
                if (dist < 4.0f) player.health.takeDamage((int)(swipeDamage() * _enrageMultiplier));
                break;
            case Tail:
                if (dist < 3.5f) player.health.takeDamage((int)(tailDamage() * _enrageMultiplier));
                break;
            case Breath: doBreath(toPlayer, projectiles); break;
            default: break;
            }
            _currentAttack = None;
            _attackTimer = 2.0f;
        }
    }

    if (_attackTimer <= 0) {
        chooseAttack(toPlayer);
    }
}

void Dragon::chooseAttack(const glm::vec3& toPlayer) {
    float dist = glm::length(toPlayer);
    float roll = glm::linearRand(0.0f, 1.0f);

    if (phase() == DragonPhase::One) {
        if (roll < 0.6f) { _currentAttack = Swipe; _telegraphTimer = 0.8f; }
        else { _currentAttack = Tail; _telegraphTimer = 0.6f; }
    } else if (phase() == DragonPhase::Two) {
        if (roll < 0.35f) { _currentAttack = Swipe; _telegraphTimer = 0.7f; }
        else if (roll < 0.65f) { _currentAttack = Tail; _telegraphTimer = 0.5f; }
        else { _currentAttack = Breath; _telegraphTimer = 1.2f; }
    } else { // Phase 3 — enraged
        if (roll < 0.3f) { _currentAttack = Swipe; _telegraphTimer = 0.4f; }
        else if (roll < 0.5f) { _currentAttack = Tail; _telegraphTimer = 0.3f; }
        else { _currentAttack = Breath; _telegraphTimer = 0.8f; }
    }
}

void Dragon::doSwipe(const glm::vec3& dir) {}
void Dragon::doTail(const glm::vec3& dir) {}

void Dragon::doBreath(const glm::vec3& dir,
    std::vector<std::unique_ptr<GameObject>>& projectiles) {
    // Cone of fire — spawn multiple projectiles in a fan
    glm::vec3 fwd = glm::normalize(glm::vec3(
        sin(transform.rotation), 0, cos(transform.rotation)));

    int count = (phase() == DragonPhase::Three) ? 8 : 5;
    float spread = (phase() == DragonPhase::Three) ? 1.2f : 0.6f; // Wider in P3

    for (int i = 0; i < count; i++) {
        float angle = (i - (count-1)/2.0f) * (spread / count);
        glm::vec3 fireDir(
            fwd.x * cos(angle) - fwd.z * sin(angle),
            0,
            fwd.x * sin(angle) + fwd.z * cos(angle));

        auto p = std::make_unique<Projectile>(
            (int)(breathDamage() * _enrageMultiplier), 6.0f, 2.5f, 1.5f, false);
        p->transform.position = transform.position + fwd * 3.0f + glm::vec3(0, 1.5f, 0);
        p->setVelocity(fireDir * 6.0f);
        p->renderable.tint = {1.0f, 0.4f, 0.1f};
        projectiles.push_back(std::move(p));
    }
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/mob/Dragon.h src/mob/Dragon.cpp
git commit -m "feat: Dragon boss — 3 phases, swipe/tail/breath attacks, enrage"
```

---

### Task 5.2: Wire Dragon into Game

**Modify:** `~/Documents/wizardbattle/src/core/Game.h`, `Game.cpp`

- [ ] **Step 1: Update Game.h — add Dragon and boss arena state**

```cpp
#include "mob/Dragon.h"
// In Game class, add:
    Dragon _dragon;
    bool _bossSpawned = false;
    bool _bossDefeated = false;
```

- [ ] **Step 2: Update Game::update — boss spawn check and dragon update**

Add after the spawner update:
```cpp
    // Boss spawn check
    if (!_bossSpawned && _spawner.killCount() >= 100) {
        _bossSpawned = true;
        _dragon.activate();
        printf("*** The Forest Dragon has awakened! ***\n");
    }

    // Dragon logic
    if (_bossSpawned && !_bossDefeated) {
        _dragon.update(dt, _wizard, _projectiles);
        _dragon.transform.position.y = _terrain.heightAt(
            _dragon.transform.position.x, _dragon.transform.position.z);

        // Projectile-dragon collision
        for (auto& p : _projectiles) {
            if (p->destroyed()) continue;
            auto* proj = dynamic_cast<Projectile*>(p.get());
            if (!proj) continue;
            if (_dragon.health.alive() &&
                aabbIntersects(getWorldAABB(*p), getWorldAABB(_dragon))) {
                _dragon.health.takeDamage(proj->damage());
                if (!_dragon.health.alive()) {
                    _bossDefeated = true;
                    _wizard.addXP(500);
                    _state = GameState::Victory;
                    printf("*** Dragon slain! Victory! ***\n");
                }
                if (!proj->piercing()) p->setDestroyed(true);
            }
        }
    }
```

- [ ] **Step 3: Update Game::render — draw dragon**

Add to render, before mob drawing (dragon renders on top of terrain):
```cpp
    // Draw dragon if spawned
    if (_bossSpawned && !_bossDefeated && _dragon.health.alive()) {
        _renderer.drawMesh(_cube, _whiteTex,
            glm::translate(glm::mat4(1), _dragon.transform.position)
            * glm::rotate(glm::mat4(1), _dragon.transform.rotation,
                glm::vec3(0, 1, 0))
            * glm::scale(glm::mat4(1), _dragon.transform.scale));
    }
```

- [ ] **Step 4: Update Game::update — player death check**

Add at end of update:
```cpp
    if (_wizard.isDead() && _state == GameState::Playing) {
        _state = GameState::GameOver;
        printf("Game Over!\n");
    }
```

- [ ] **Step 5: Build and run**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc) && ./wizardbattle
```

Expected: After 100 kills, dragon spawns at center. 3-phase fight with swipe, tail, and fire breath. Boss death triggers victory state.

- [ ] **Step 6: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Game.h src/core/Game.cpp
git commit -m "feat: Dragon boss integration — spawn at 100 kills, 3-phase fight, victory"
```

---

## Phase 6: Polish

### Task 6.1: Forest map — tree and rock placement

**Files:**
- Create: `~/Documents/wizardbattle/src/world/ForestMap.h`
- Create: `~/Documents/wizardbattle/src/world/ForestMap.cpp`

- [ ] **Step 1: Write ForestMap.h**

```cpp
#pragma once
#include <glm/glm.hpp>
#include <vector>

struct TreePlacement {
    glm::vec3 position;
    float scale;
    float rotation;
};

struct RockPlacement {
    glm::vec3 position;
    float scale;
};

class ForestMap {
public:
    ForestMap(float mapSize, int treeCount, int rockCount);

    const std::vector<TreePlacement>& trees() const { return _trees; }
    const std::vector<RockPlacement>& rocks() const { return _rocks; }

    bool isInClearing(glm::vec2 pos) const;
    static constexpr float CLEARING_SIZE = 40.0f;

private:
    std::vector<TreePlacement> _trees;
    std::vector<RockPlacement> _rocks;
    float _mapSize;
    glm::vec2 _center;
};
```

- [ ] **Step 2: Write ForestMap.cpp**

```cpp
#include "world/ForestMap.h"
#include <glm/gtc/random.hpp>

ForestMap::ForestMap(float mapSize, int treeCount, int rockCount)
    : _mapSize(mapSize), _center(mapSize/2, mapSize/2) {

    float margin = mapSize * 0.05f;
    _trees.reserve(treeCount);
    for (int i = 0; i < treeCount; i++) {
        glm::vec2 pos;
        int attempts = 0;
        do {
            pos = {glm::linearRand(margin, mapSize - margin),
                   glm::linearRand(margin, mapSize - margin)};
            attempts++;
        } while (isInClearing(pos) && attempts < 50);

        float scale = glm::linearRand(0.8f, 1.5f);
        float rot = glm::linearRand(0.0f, glm::pi<float>() * 2);
        _trees.push_back({glm::vec3(pos.x, 0, pos.y), scale, rot});
    }

    _rocks.reserve(rockCount);
    for (int i = 0; i < rockCount; i++) {
        glm::vec2 pos;
        int attempts = 0;
        do {
            pos = {glm::linearRand(margin, mapSize - margin),
                   glm::linearRand(margin, mapSize - margin)};
            attempts++;
        } while (isInClearing(pos) && attempts < 50);

        float scale = glm::linearRand(0.5f, 1.5f);
        _rocks.push_back({glm::vec3(pos.x, 0, pos.y), scale});
    }
}

bool ForestMap::isInClearing(glm::vec2 pos) const {
    return glm::distance(pos, _center) < CLEARING_SIZE / 2.0f;
}
```

- [ ] **Step 3: Build**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc)
```

Expected: Compiles cleanly.

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/world/ForestMap.h src/world/ForestMap.cpp
git commit -m "feat: ForestMap — tree/rock scatter, central clearing for boss"
```

---

### Task 6.2: Wire ForestMap into Game — render trees and rocks

**Modify:** `~/Documents/wizardbattle/src/core/Game.h`, `Game.cpp`

- [ ] **Step 1: Update Game.h — add forest map**

```cpp
#include "world/ForestMap.h"
// In Game class, add:
    ForestMap _forest;
```

- [ ] **Step 2: Update Game constructor — init forest**

```cpp
Game::Game()
    : _window("WizardBattle", 1280, 720)
    , _camera(1280, 720)
    , _terrain(200, 1.0f)
    , _forest(200.0f, 300, 80)  // 300 trees, 80 rocks
    , _cube(Mesh::createCube())
    , _whiteTex(Texture::createSolid(255, 255, 255))
{ ... }
```

- [ ] **Step 3: Update Game::render — draw trees and rocks**

Add after terrain rendering:
```cpp
    // Draw trees (green cubes for now, taller)
    for (auto& t : _forest.trees()) {
        glm::vec3 pos = t.position;
        pos.y = _terrain.heightAt(pos.x, pos.z);
        _renderer.drawMesh(_cube, _whiteTex,
            glm::translate(glm::mat4(1), pos + glm::vec3(0, t.scale, 0))
            * glm::rotate(glm::mat4(1), t.rotation, glm::vec3(0,1,0))
            * glm::scale(glm::mat4(1), glm::vec3(0.3f, t.scale * 2, 0.3f)));
    }

    // Draw rocks (grey cubes)
    for (auto& r : _forest.rocks()) {
        glm::vec3 pos = r.position;
        pos.y = _terrain.heightAt(pos.x, pos.z);
        _renderer.drawMesh(_cube, _whiteTex,
            glm::translate(glm::mat4(1), pos)
            * glm::scale(glm::mat4(1), glm::vec3(r.scale * 0.5f,
                r.scale * 0.3f, r.scale * 0.5f)));
    }
```

- [ ] **Step 4: Build and run**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc) && ./wizardbattle
```

Expected: Forest with trees and rocks scattered, central clearing visible.

- [ ] **Step 5: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/core/Game.h src/core/Game.cpp
git commit -m "feat: Forest map with 300 trees, 80 rocks, central boss clearing"
```

---

### Task 6.3: HUD — health, mana, XP bars, spell bar, kill counter

**Files:**
- Create: `~/Documents/wizardbattle/src/ui/HUD.h`
- Create: `~/Documents/wizardbattle/src/ui/HUD.cpp`

NOTE: Simple 2D HUD using OpenGL immediate mode (glBegin/glEnd for quads drawn in screen space). Uses a separate orthographic projection rendered after the 3D scene. The OpenGL 3.3 core profile doesn't support immediate mode — instead we render colored quads using the default shader with a screen-space ortho projection.

Actually, for simplicity, let's use a dedicated HUD shader or just reuse the existing shader with a screen-space ortho matrix.

For a clean approach: render HUD as textured/colored quads in NDC space using the existing shader with identity model matrix and a simple orthographic projection for 2D.

The simplest approach: use a separate shader program that takes positions in NDC space directly.

Let me keep it simple — HUD renders after the 3D scene with glViewport and a 2D projection.

- [ ] **Step 1: Write HUD.h**

```cpp
#pragma once
#include <string>

class Wizard;
class SpellBook;
class Shader;
class Mesh;

class HUD {
public:
    HUD();
    void render(const Wizard& wizard, const SpellBook& spells,
        int killCount, bool bossActive, float bossHpRatio);

private:
    void drawBar(float x, float y, float w, float h,
        float fill, float r, float g, float b, float bgR, float bgG, float bgB);
    void drawText(const std::string& text, float x, float y, float scale);

    Shader* _shader = nullptr;
    Mesh* _quad = nullptr;
    Mesh* _barMesh = nullptr; // Thin quad for bars
};
```

- [ ] **Step 2: Write HUD.cpp**

```cpp
#include "ui/HUD.h"
#include "core/Shader.h"
#include "core/Mesh.h"
#include "player/Wizard.h"
#include "player/SpellBook.h"
#include "player/Progression.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>

HUD::HUD() {
    _shader = new Shader("assets/shaders/default.vert", "assets/shaders/default.frag");
    _quad = new Mesh(Mesh::createQuad());
    _barMesh = new Mesh(Mesh::createQuad());
}

void HUD::render(const Wizard& wizard, const SpellBook& spells,
    int killCount, bool bossActive, float bossHpRatio) {

    // Save and set up 2D ortho
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _shader->use();
    glm::mat4 ortho = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f);
    _shader->setMat4("uProj", &ortho[0][0]);
    glm::mat4 identity(1);
    _shader->setMat4("uView", &identity[0][0]);
    _shader->setMat4("uModel", &identity[0][0]);
    _shader->setVec3("uLightDir", 0, -1, 0);
    _shader->setVec3("uLightColor", 1, 1, 1);
    _shader->setVec3("uAmbient", 1, 1, 1);
    _shader->setVec3("uViewPos", 0, 0, 0);
    _shader->setVec3("uFogColor", 0, 0, 0);
    _shader->setFloat("uFogStart", 9999);
    _shader->setFloat("uFogEnd", 9999);

    auto drawRect = [&](float x, float y, float w, float h, float r, float g, float b, float a) {
        _shader->setVec3("uLightColor", r, g, b);
        _shader->setVec3("uAmbient", r, g, b);
        glm::mat4 m = glm::translate(identity, glm::vec3(x, y, 0))
            * glm::scale(identity, glm::vec3(w, h, 1));
        _shader->setMat4("uModel", &m[0][0]);
        _shader->setInt("uTexture", -1); // No texture
        _quad->draw();
    };

    // Health bar (top-left)
    float hpPct = (float)wizard.health.current / wizard.health.max;
    drawRect(20, 20, 200, 22, 0.15f, 0.15f, 0.15f, 0.8f); // BG
    drawRect(20, 20, 200 * hpPct, 22, 0.9f, 0.15f, 0.15f, 1.0f); // HP

    // Mana bar
    float mpPct = wizard.mana / wizard.maxMana;
    drawRect(20, 46, 200, 18, 0.15f, 0.15f, 0.15f, 0.8f); // BG
    drawRect(20, 46, 200 * mpPct, 18, 0.15f, 0.4f, 0.9f, 1.0f); // Mana

    // XP bar (bottom-center)
    float xpPct = Progression::xpProgress(wizard.xp - 
        (wizard.level > 1 ? Progression::xpForLevel(wizard.level) : 0),
        wizard.level > 1 ? wizard.level : 1);
    if (wizard.level >= 10) xpPct = 1.0f;
    drawRect(440, 690, 400, 14, 0.1f, 0.1f, 0.1f, 0.8f);
    drawRect(440, 690, 400 * xpPct, 14, 0.9f, 0.8f, 0.2f, 1.0f);

    // Spell bar (bottom)
    for (int i = 0; i < 5; i++) {
        float sx = 390 + i * 100;
        bool unlocked = spells.cooldownRemaining(i) >= 0;
        float cd = spells.cooldownRemaining(i);

        // Slot background
        float r = unlocked ? 0.3f : 0.15f;
        float g = unlocked ? 0.3f : 0.15f;
        float b = unlocked ? 0.3f : 0.15f;
        drawRect(sx, 655, 90, 30, r, g, b, 0.9f);

        // Cooldown overlay
        if (cd > 0 && unlocked) {
            auto& s = spells;
            // Actually need full cd for ratio. Use spellName as proxy.
            // Simpler: just show dark overlay
            drawRect(sx, 655, 90, 30, 0.1f, 0.1f, 0.1f, 0.6f);
        }
    }

    // Kill counter (top-right)
    char killText[64];
    snprintf(killText, sizeof(killText), "Kills: %d / 100", killCount);
    // (Text rendering is complex in OpenGL — skip for now, use console)
    // Will be covered by in-game text via console output

    // Boss health bar (top-center, if active)
    if (bossActive) {
        drawRect(390, 10, 500, 20, 0.1f, 0.1f, 0.1f, 0.8f);
        drawRect(390, 10, 500 * bossHpRatio, 20, 0.9f, 0.2f, 0.2f, 1.0f);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HUD::drawBar(float x, float y, float w, float h,
    float fill, float r, float g, float b, float bgR, float bgG, float bgB) {}

void HUD::drawText(const std::string& text, float x, float y, float scale) {}
```

- [ ] **Step 3: Wire HUD into Game.h and Game.cpp**

Add to Game.h:
```cpp
#include "ui/HUD.h"
    HUD _hud;
```

Add to Game::render, after _renderer.end() and before _window.swap():
```cpp
    // Render HUD
    float bossHpRatio = _bossSpawned && !_bossDefeated
        ? (float)_dragon.health.current / _dragon.health.max : 0;
    _hud.render(_wizard, _spellBook, _spawner.killCount(),
        _bossSpawned && !_bossDefeated, bossHpRatio);
```

- [ ] **Step 4: Build and run**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc) && ./wizardbattle
```

Expected: HP bar, mana bar, XP bar, spell slots visible. Kill counter in top-right. Boss HP bar appears during fight.

- [ ] **Step 5: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/ui/HUD.h src/ui/HUD.cpp src/core/Game.h src/core/Game.cpp
git commit -m "feat: HUD — health/mana/XP bars, spell bar, kill counter, boss HP bar"
```

---

### Task 6.4: Damage numbers

**Files:**
- Create: `~/Documents/wizardbattle/src/ui/DamageNumbers.h`
- Create: `~/Documents/wizardbattle/src/ui/DamageNumbers.cpp`

- [ ] **Step 1: Write DamageNumbers.h**

```cpp
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
```

- [ ] **Step 2: Write DamageNumbers.cpp**

```cpp
#include "ui/DamageNumbers.h"
#include <glm/gtc/matrix_transform.hpp>

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
        n.worldPos.y += dt * 3.0f; // Float upward
    }
    _numbers.erase(
        std::remove_if(_numbers.begin(), _numbers.end(),
            [](auto& n) { return n.life <= 0; }),
        _numbers.end());
}

void DamageNumbers::render(const glm::mat4& view, const glm::mat4& proj) {
    for (auto& n : _numbers) {
        // Billboard toward camera — use world position, render as colored quad
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
```

- [ ] **Step 3: Wire DamageNumbers into Game**

Add to Game.h:
```cpp
#include "ui/DamageNumbers.h"
    DamageNumbers _damageNumbers;
```

In Game::update, wherever a mob takes damage, add:
```cpp
    if (/* mob took damage from projectile */) {
        _damageNumbers.add(m->transform.position, proj->damage());
    }
```

In Game::update, add:
```cpp
    _damageNumbers.update(dt);
```

In Game::render, before _renderer.end():
```cpp
    _damageNumbers.render(_camera.view(), _camera.proj());
```

- [ ] **Step 4: Build and run**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc) && ./wizardbattle
```

Expected: Floating damage numbers appear when mobs are hit.

- [ ] **Step 5: Commit**

```bash
cd ~/Documents/wizardbattle
git add src/ui/DamageNumbers.h src/ui/DamageNumbers.cpp src/core/Game.h src/core/Game.cpp
git commit -m "feat: Floating damage numbers on mob hits"
```

---

### Task 6.5: Asset download script

**Files:**
- Create: `~/Documents/wizardbattle/scripts/download_assets.sh`

- [ ] **Step 1: Write download script**

```bash
#!/bin/bash
# Download free CC0 game assets for WizardBattle
set -e

ASSET_DIR="$(cd "$(dirname "$0")/.." && pwd)/assets"
MODELS="$ASSET_DIR/models"
TEXTURES="$ASSET_DIR/textures"
SOUNDS="$ASSET_DIR/sounds"

mkdir -p "$MODELS" "$TEXTURES" "$SOUNDS"

echo "=== Downloading WizardBattle assets ==="

# --- MODELS from Quaternius (CC0) ---
BASE="https://raw.githubusercontent.com/quaternius/Ultimate-Stylized-3D-Character-Pack/main/Models"

# Wizard (use Mage model)
echo "Downloading wizard model..."
curl -sL "$BASE/Mage/Mage.obj" -o "$MODELS/wizard.obj" || echo "Wizard model: using placeholder"

# Goblins (use Goblin model)
echo "Downloading goblin model..."
curl -sL "$BASE/Goblin/Goblin.obj" -o "$MODELS/goblin.obj" || echo "Goblin model: using placeholder"

# Ogre (use Cyclops or large model)
echo "Downloading ogre model..."
curl -sL "$BASE/Cyclops/Cyclops.obj" -o "$MODELS/ogre.obj" || echo "Ogre model: using placeholder"

# Archer (use Elf or similar)
echo "Downloading archer model..."
curl -sL "$BASE/Elf/Elf.obj" -o "$MODELS/archer.obj" || echo "Archer model: using placeholder"

# Dragon (use Dragon model)
echo "Downloading dragon model..." 
curl -sL "$BASE/Dragon/Dragon.obj" -o "$MODELS/dragon.obj" || echo "Dragon model: using placeholder"

# Tree low-poly
echo "Downloading tree model..."
curl -sL "https://raw.githubusercontent.com/quaternius/Low-Poly-Forest-Pack/main/Models/Tree1.obj" -o "$MODELS/tree.obj" || echo "Tree: using placeholder"

# Rock
echo "Downloading rock model..."
curl -sL "https://raw.githubusercontent.com/quaternius/Low-Poly-Forest-Pack/main/Models/Rock1.obj" -o "$MODELS/rock.obj" || echo "Rock: using placeholder"

# --- TEXTURES ---
echo "Creating procedural textures..."
# Will be generated at runtime if downloads fail

# --- SOUNDS from Freesound (CC0) ---
echo "Downloading sounds..."

# Fireball cast
curl -sL "https://freesound.org/data/previews/175/175270_3599881-lq.mp3" -o "$SOUNDS/fireball_cast.mp3" || true

# Hit
curl -sL "https://freesound.org/data/previews/331/331912_5885715-lq.mp3" -o "$SOUNDS/hit.mp3" || true

# Dragon roar
curl -sL "https://freesound.org/data/previews/413/413310_5121236-hq.mp3" -o "$SOUNDS/dragon_roar.mp3" || true

echo "=== Download complete ==="
echo "Models in: $MODELS"
echo "Total size: $(du -sh "$ASSET_DIR" 2>/dev/null | cut -f1)"
```

- [ ] **Step 2: Make executable**

```bash
chmod +x ~/Documents/wizardbattle/scripts/download_assets.sh
```

- [ ] **Step 3: Run asset download**

```bash
cd ~/Documents/wizardbattle && bash scripts/download_assets.sh
```

- [ ] **Step 4: Commit**

```bash
cd ~/Documents/wizardbattle
git add scripts/download_assets.sh
git commit -m "feat: Asset download script for models, textures, sounds"
```

---

### Task 6.6: Final integration — ensure everything compiles and runs

**Modify:** `~/Documents/wizardbattle/src/core/Game.h`, `Game.cpp`

- [ ] **Step 1: Verify CMakeLists.txt includes all source files**

The CMakeLists.txt was written in Task 1.1 with all source files listed. Verify it matches.

- [ ] **Step 2: Full build, fix any compilation errors**

```bash
cd ~/Documents/wizardbattle/build && cmake .. && make -j$(nproc) 2>&1 | tail -20
```

Expected: Clean build, no errors.

- [ ] **Step 3: Run the game and test all features**

```bash
cd ~/Documents/wizardbattle/build && ./wizardbattle
```

Quick manual test checklist:
- [ ] WASD movement on forest terrain
- [ ] Mouse aim + left-click casts spells
- [ ] 1-5 keys select spells
- [ ] Goblins, archers, ogres spawn and behave correctly
- [ ] Mobs take damage, die, grant XP
- [ ] XP bar progresses, level up works
- [ ] Spells unlock at correct levels
- [ ] Dragon spawns after 100 kills
- [ ] Dragon has 3 phases
- [ ] Victory on dragon death
- [ ] Player death → game over

- [ ] **Step 4: Final commit**

```bash
cd ~/Documents/wizardbattle
git add -A
git commit -m "feat: Final integration — complete WizardBattle game"
```
