#pragma once
#include "entity/GameObject.h"
#include <vector>
#include <memory>

enum class DragonPhase { One, Two, Three };

class Dragon : public GameObject {
public:
    Dragon();
    void update(float dt, GameObject& player,
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
