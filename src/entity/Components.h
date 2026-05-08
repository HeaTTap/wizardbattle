#pragma once
#include <glm/glm.hpp>
#include <functional>

struct Transform {
    glm::vec3 position{0};
    glm::vec3 scale{1};
    float rotation = 0;
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
    int meshId = -1;
    int textureId = -1;
    glm::vec3 tint{1};
    bool visible = true;
};

struct Collider {
    glm::vec3 halfExtents{0.5f};
    bool isTrigger = false;
    bool solid = true;
    std::function<void(class GameObject*)> onCollide;
};
