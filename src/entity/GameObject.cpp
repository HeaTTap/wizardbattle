#include "entity/GameObject.h"

int GameObject::_nextId = 0;

GameObject::GameObject(const std::string& tag)
    : _tag(tag), _id(_nextId++) {}
