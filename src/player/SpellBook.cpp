#include "player/SpellBook.h"
#include <glm/gtc/constants.hpp>
#include <memory>

class IceShard : public Spell {
public:
    IceShard() : Spell("Ice Shard", 1.5f, 25.0f, 20) {}
    void cast(glm::vec3 origin, glm::vec3 direction,
        std::vector<std::unique_ptr<GameObject>>& projectiles) override {
        auto p = std::make_unique<Projectile>(_damage, 12.0f, 3.0f, 0, true);
        p->transform.position = origin + glm::vec3(0, 1.0f, 0);
        p->setVelocity(direction * 12.0f);
        p->renderable.tint = {0.4f, 0.7f, 1.0f};
        projectiles.push_back(std::move(p));
    }
};

class LightningStrike : public Spell {
public:
    LightningStrike() : Spell("Lightning Strike", 3.0f, 35.0f, 45) {}
    void cast(glm::vec3 origin, glm::vec3 direction,
        std::vector<std::unique_ptr<GameObject>>& projectiles) override {
        for (int i = 0; i < 3; i++) {
            float offsetX = (i - 1) * 3.0f;
            auto p = std::make_unique<Projectile>(_damage, 50.0f, 0.3f, 3.0f, false);
            p->transform.position = origin + glm::vec3(offsetX, 1.0f, 0);
            p->setVelocity(direction * 50.0f);
            p->renderable.tint = {1.0f, 1.0f, 0.3f};
            projectiles.push_back(std::move(p));
        }
    }
};

class ArcaneNova : public Spell {
public:
    ArcaneNova() : Spell("Arcane Nova", 6.0f, 50.0f, 30) {}
    void cast(glm::vec3 origin, glm::vec3 direction,
        std::vector<std::unique_ptr<GameObject>>& projectiles) override {
        for (int i = 0; i < 8; i++) {
            float angle = i * glm::pi<float>() / 4.0f;
            glm::vec3 dir(cos(angle), 0, sin(angle));
            auto p = std::make_unique<Projectile>(_damage, 8.0f, 1.5f, 4.0f, false);
            p->transform.position = origin + glm::vec3(0, 1.0f, 0);
            p->setVelocity(dir * 8.0f);
            p->renderable.tint = {0.7f, 0.3f, 1.0f};
            projectiles.push_back(std::move(p));
        }
    }
};

class Meteor : public Spell {
public:
    Meteor() : Spell("Meteor", 12.0f, 80.0f, 100) {}
    void cast(glm::vec3 origin, glm::vec3 direction,
        std::vector<std::unique_ptr<GameObject>>& projectiles) override {
        glm::vec3 target = origin + direction * 15.0f;
        auto p = std::make_unique<Projectile>(_damage, 0, 1.0f, 8.0f, false);
        p->transform.position = target + glm::vec3(0, 20.0f, 0);
        p->setVelocity(glm::vec3(0, -20.0f, 0));
        p->renderable.tint = {1.0f, 0.3f, 0.1f};
        projectiles.push_back(std::move(p));
    }
};

SpellBook::SpellBook() {
    _spells.push_back(std::make_unique<Spell>("Firebolt", 0.8f, 15.0f, 25));
    _spells.push_back(std::make_unique<IceShard>());
    _spells.push_back(std::make_unique<LightningStrike>());
    _spells.push_back(std::make_unique<ArcaneNova>());
    _spells.push_back(std::make_unique<Meteor>());

    _spells[0]->setUnlockLevel(1);
    _spells[1]->setUnlockLevel(3);
    _spells[2]->setUnlockLevel(5);
    _spells[3]->setUnlockLevel(8);
    _spells[4]->setUnlockLevel(10);

    _cooldowns.resize(5, 0);
}

void SpellBook::update(float dt) {
    for (auto& cd : _cooldowns)
        if (cd > 0) cd -= dt;
}

bool SpellBook::cast(int index, glm::vec3 origin, glm::vec3 direction,
    float& mana, std::vector<std::unique_ptr<GameObject>>& projectiles) {

    if (index < 0 || index >= 5) return false;
    if (!isUnlocked(index)) return false;
    auto& s = _spells[index];
    if (_cooldowns[index] > 0) return false;
    if (mana < s->manaCost()) return false;

    s->cast(origin, direction, projectiles);
    mana -= s->manaCost();
    _cooldowns[index] = s->cooldown();
    return true;
}

bool SpellBook::isUnlocked(int index) const {
    return index < (int)_spells.size();
}

void SpellBook::unlockForLevel(int level) {}

float SpellBook::cooldownRemaining(int index) const {
    if (index < 0 || index >= 5) return 0;
    if (!isUnlocked(index)) return -1;
    return _cooldowns[index];
}

float SpellBook::manaCost(int index) const {
    if (index < 0 || index >= 5) return 0;
    return _spells[index]->manaCost();
}

const std::string& SpellBook::spellName(int index) const {
    static std::string locked = "Locked";
    if (index < 0 || index >= 5) return locked;
    return _spells[index]->name();
}

void SpellBook::selectSpell(int index) {
    if (isUnlocked(index)) _selected = index;
}
