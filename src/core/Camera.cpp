#include "core/Camera.h"
#include <cmath>

Camera::Camera(int screenW, int screenH) : _screenW(screenW), _screenH(screenH) {}

void Camera::update(float dt) {
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
