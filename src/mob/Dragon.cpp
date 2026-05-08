#include "mob/Dragon.h"
#include "player/Spell.h"
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

Dragon::Dragon() : GameObject("Dragon") {
    transform.position = {100, 0, 100};
    transform.scale = {3.0f, 3.0f, 3.0f};
    health.current = 2000;
    health.max = 2000;
    collider.halfExtents = {2.0f, 2.5f, 2.0f};
    collider.solid = true;
    renderable.tint = {0.8f, 0.2f, 0.1f};
}

DragonPhase Dragon::phase() const {
    float ratio = (float)health.current / health.max;
    if (ratio > 0.6f) return DragonPhase::One;
    if (ratio > 0.3f) return DragonPhase::Two;
    return DragonPhase::Three;
}

void Dragon::update(float dt, GameObject& player,
    std::vector<std::unique_ptr<GameObject>>& projectiles) {

    if (!_active) return;
    if (!health.alive()) return;

    _enrageMultiplier = (phase() == DragonPhase::Three) ? 1.5f : 1.0f;
    float speed = _enrageMultiplier;

    glm::vec3 toPlayer = player.transform.position - transform.position;
    float dist = glm::length(toPlayer);

    if (dist > 0.01f) {
        glm::vec3 dir = toPlayer / dist;
        float targetAngle = atan2(dir.x, dir.z);
        float da = targetAngle - transform.rotation;
        while (da > glm::pi<float>()) da -= 2 * glm::pi<float>();
        while (da < -glm::pi<float>()) da += 2 * glm::pi<float>();
        transform.rotation += da * glm::min(dt * 3.0f, 1.0f);
    }

    _attackTimer -= dt * speed;

    if (_currentAttack != None) {
        _telegraphTimer -= dt * speed;
        if (_telegraphTimer <= 0) {
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
    float roll = glm::linearRand(0.0f, 1.0f);

    if (phase() == DragonPhase::One) {
        if (roll < 0.6f) { _currentAttack = Swipe; _telegraphTimer = 0.8f; }
        else { _currentAttack = Tail; _telegraphTimer = 0.6f; }
    } else if (phase() == DragonPhase::Two) {
        if (roll < 0.35f) { _currentAttack = Swipe; _telegraphTimer = 0.7f; }
        else if (roll < 0.65f) { _currentAttack = Tail; _telegraphTimer = 0.5f; }
        else { _currentAttack = Breath; _telegraphTimer = 1.2f; }
    } else {
        if (roll < 0.3f) { _currentAttack = Swipe; _telegraphTimer = 0.4f; }
        else if (roll < 0.5f) { _currentAttack = Tail; _telegraphTimer = 0.3f; }
        else { _currentAttack = Breath; _telegraphTimer = 0.8f; }
    }
}

void Dragon::doSwipe(const glm::vec3& dir) {}
void Dragon::doTail(const glm::vec3& dir) {}

void Dragon::doBreath(const glm::vec3& dir,
    std::vector<std::unique_ptr<GameObject>>& projectiles) {
    glm::vec3 fwd = glm::normalize(glm::vec3(
        sin(transform.rotation), 0, cos(transform.rotation)));

    int count = (phase() == DragonPhase::Three) ? 8 : 5;
    float spread = (phase() == DragonPhase::Three) ? 1.2f : 0.6f;

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
