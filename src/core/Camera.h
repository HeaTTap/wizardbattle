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
