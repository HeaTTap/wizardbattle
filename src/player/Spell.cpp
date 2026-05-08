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
    renderable.tint = {1.0f, 0.5f, 0.0f};
}

void Projectile::update(float dt) {
    transform.position += _velocity * dt;
    _life -= dt;
    if (_life <= 0) setDestroyed(true);
}
