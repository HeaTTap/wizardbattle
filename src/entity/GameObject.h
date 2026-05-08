#pragma once
#include "entity/Components.h"
#include <string>

class GameObject {
public:
    GameObject(const std::string& tag = "");
    virtual ~GameObject() = default;

    virtual void update(float) {}
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
