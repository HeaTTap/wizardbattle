#pragma once
#include "entity/GameObject.h"
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>

class Spell {
public:
    Spell(const std::string& name, float cooldown, float manaCost, int dmg);
    virtual ~Spell() = default;

    virtual void cast(glm::vec3 origin, glm::vec3 direction,
        std::vector<std::unique_ptr<GameObject>>& projectiles);

    const std::string& name() const { return _name; }
    float cooldown() const { return _cooldown; }
    float manaCost() const { return _manaCost; }
    int damage() const { return _damage; }
    int unlockLevel() const { return _unlockLevel; }
    void setUnlockLevel(int lvl) { _unlockLevel = lvl; }

protected:
    std::string _name;
    float _cooldown;
    float _manaCost;
    int _damage;
    int _unlockLevel = 1;
};

class Projectile : public GameObject {
public:
    Projectile(int dmg, float speed, float lifetime,
        float aoeRadius = 0, bool piercing = false);

    void update(float dt) override;

    int damage() const { return _dmg; }
    float aoeRadius() const { return _aoe; }
    bool piercing() const { return _pierce; }
    float lifespan() const { return _life; }

    void setVelocity(glm::vec3 v) { _velocity = v; }

private:
    int _dmg;
    float _aoe;
    bool _pierce;
    float _life;
    float _maxLife;
    glm::vec3 _velocity{0};
};
