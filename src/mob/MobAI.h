#pragma once
#include "entity/GameObject.h"
#include "entity/Collision.h"
#include <glm/glm.hpp>

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
    float _fleeThreshold = 0.2f;

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
