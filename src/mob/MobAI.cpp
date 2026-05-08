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
            glm::vec3 away = self.transform.position - player.transform.position;
            away.y = 0;
            if (glm::length(away) > 0.01f)
                _moveDir = glm::normalize(away) * _moveSpeed;
        } else if (dist > _attackRange * 1.5f) {
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
        _moveDir = glm::normalize(away) * _moveSpeed * 1.3f;
        if (dist > _detectRadius * 2.5f)
            _state = AIState::Idle;
        break;
    }

    case AIState::Dead:
        _moveDir = glm::vec3(0);
        break;
    }
}
