#pragma once
#include "player/Spell.h"
#include <vector>
#include <memory>

class SpellBook {
public:
    SpellBook();

    void update(float dt);
    bool cast(int index, glm::vec3 origin, glm::vec3 direction,
        float& mana, std::vector<std::unique_ptr<GameObject>>& projectiles);

    int spellCount() const { return 5; }
    bool isUnlocked(int index) const;
    void unlockForLevel(int level);

    float cooldownRemaining(int index) const;
    float manaCost(int index) const;
    const std::string& spellName(int index) const;
    int selectedSpell() const { return _selected; }
    void selectSpell(int index);

private:
    std::vector<std::unique_ptr<Spell>> _spells;
    std::vector<float> _cooldowns;
    int _selected = 0;
};
