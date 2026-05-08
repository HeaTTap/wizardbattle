#include "core/Game.h"
#include <GL/glew.h>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Game::Game()
    : _window("WizardBattle", 1280, 720)
    , _camera(1280, 720)
    , _terrain(200, 1.0f)
    , _forest(200.0f, 300, 80)
    , _cube(Mesh::createCube())
    , _whiteTex(Texture::createSolid(255, 255, 255))
{
    if (_window.shouldClose()) return;
}

void Game::run() {
    if (_window.shouldClose()) return;

    auto prevTime = std::chrono::high_resolution_clock::now();
    const float fixedDt = 1.0f / 60.0f;
    float accumulator = 0;

    while (!_window.shouldClose()) {
        auto now = std::chrono::high_resolution_clock::now();
        float frameDt = std::chrono::duration<float>(now - prevTime).count();
        prevTime = now;
        accumulator += frameDt;

        _input.update();

        if (_input.keyPressed(SDLK_ESCAPE)) {
            _window.setShouldClose(true);
        }

        while (accumulator >= fixedDt) {
            update(fixedDt);
            accumulator -= fixedDt;
        }

        render();
        _window.swap();
    }
}

void Game::update(float dt) {
    if (_state != GameState::Playing) return;

    _camera.setTarget(_wizard.transform.position);
    _camera.update(dt);
    if (_input.mouseWheel() != 0)
        _camera.zoom((float)_input.mouseWheel() * 2.0f);

    glm::vec3 moveDir(0);
    if (_input.keyDown(SDLK_W)) moveDir.z -= 1;
    if (_input.keyDown(SDLK_S)) moveDir.z += 1;
    if (_input.keyDown(SDLK_A)) moveDir.x -= 1;
    if (_input.keyDown(SDLK_D)) moveDir.x += 1;
    _wizard.move(moveDir, dt);

    _wizard.aimAt(_input.mouseX(), _input.mouseY(), _camera);

    // Spell selection
    for (int i = 0; i < 5; i++) {
        if (_input.keyPressed((SDL_Keycode)(SDLK_1 + i))) {
            _spellBook.selectSpell(i);
        }
    }
    if (_input.keyDown(SDLK_LSHIFT)) {
        if (_input.mouseWheel() > 0)
            _spellBook.selectSpell((_spellBook.selectedSpell() + 4) % 5);
        if (_input.mouseWheel() < 0)
            _spellBook.selectSpell((_spellBook.selectedSpell() + 1) % 5);
    }

    // Cast selected spell
    if (_input.mousePressed(SDL_BUTTON_LEFT) &&
        _wizard.globalCooldown <= 0) {
        if (_spellBook.cast(_spellBook.selectedSpell(),
            _wizard.transform.position, _wizard.aimDirection(),
            _wizard.mana, _projectiles)) {
            _wizard.globalCooldown = 0.2f;
        }
    }

    if (_wizard.leveledUp()) {
        _wizard.clearLevelUp();
    }

    _spellBook.update(dt);

    // Spawner
    _spawner.update(dt, _wizard.transform.position, _mobs);

    // Build spatial grid for mob pathfinding
    SpatialGrid grid(200.0f, 10.0f);
    for (auto& m : _mobs) {
        if (!m->destroyed()) grid.insert(m.get());
    }

    // Update mobs
    for (auto& m : _mobs) {
        if (auto* g = dynamic_cast<Goblin*>(m.get())) g->update(dt, _wizard, grid);
        else if (auto* a = dynamic_cast<Archer*>(m.get())) a->update(dt, _wizard, grid);
        else if (auto* o = dynamic_cast<Ogre*>(m.get())) o->update(dt, _wizard, grid);
    }

    // Projectile-mob collision
    for (auto& p : _projectiles) {
        if (p->destroyed()) continue;
        auto* proj = dynamic_cast<Projectile*>(p.get());
        if (!proj) continue;

        for (auto& m : _mobs) {
            if (!m->health.alive()) continue;
            if (aabbIntersects(getWorldAABB(*p), getWorldAABB(*m))) {
                m->health.takeDamage(proj->damage());
                _damageNumbers.add(m->transform.position, proj->damage());

                if (!m->health.alive()) {
                    _spawner.addKill();
                    int xp = 0;
                    if (dynamic_cast<Goblin*>(m.get())) xp = 30;
                    else if (dynamic_cast<Archer*>(m.get())) xp = 45;
                    else if (dynamic_cast<Ogre*>(m.get())) xp = 70;
                    _wizard.addXP(xp);
                }

                if (!proj->piercing())
                    p->setDestroyed(true);
                break;
            }
        }
    }

    // Mob melee attacks on player
    for (auto& m : _mobs) {
        if (!m->health.alive()) continue;
        MobAI* ai = nullptr;
        if (auto* g = dynamic_cast<Goblin*>(m.get())) ai = &g->ai;
        else if (auto* a = dynamic_cast<Archer*>(m.get())) ai = &a->ai;
        else if (auto* o = dynamic_cast<Ogre*>(m.get())) ai = &o->ai;
        if (!ai) continue;

        if (ai->wantsToAttack()) {
            float dist = glm::distance(
                glm::vec2(m->transform.position.x, m->transform.position.z),
                glm::vec2(_wizard.transform.position.x, _wizard.transform.position.z));
            if (dist < 1.8f) {
                _wizard.takeDamage(ai->attackDamage());
            }
            ai->resetCooldown();
            ai->clearAttack();
        }

        if (ai->wantsProjectile()) {
            auto arrow = std::make_unique<Projectile>(
                ai->attackDamage(), 10.0f, 3.0f, 0, false);
            arrow->transform.position = m->transform.position + glm::vec3(0,1,0);
            glm::vec3 dir = glm::normalize(
                ai->projectileTarget() - m->transform.position);
            arrow->setVelocity(dir * 10.0f);
            arrow->renderable.tint = {0.5f, 0.3f, 0.2f};
            _projectiles.push_back(std::move(arrow));
            ai->resetCooldown();
            ai->clearProjectile();
        }
    }

    // Boss spawn check
    if (!_bossSpawned && _spawner.killCount() >= 100) {
        _bossSpawned = true;
        _dragon.activate();
        printf("*** The Forest Dragon has awakened! ***\n");
    }

    // Dragon logic
    if (_bossSpawned && !_bossDefeated) {
        _dragon.update(dt, _wizard, _projectiles);
        _dragon.transform.position.y = _terrain.heightAt(
            _dragon.transform.position.x, _dragon.transform.position.z);

        for (auto& p : _projectiles) {
            if (p->destroyed()) continue;
            auto* proj = dynamic_cast<Projectile*>(p.get());
            if (!proj) continue;
            if (_dragon.health.alive() &&
                aabbIntersects(getWorldAABB(*p), getWorldAABB(_dragon))) {
                _dragon.health.takeDamage(proj->damage());
                if (!_dragon.health.alive()) {
                    _bossDefeated = true;
                    _wizard.addXP(500);
                    _state = GameState::Victory;
                    printf("*** Dragon slain! Victory! ***\n");
                }
                if (!proj->piercing()) p->setDestroyed(true);
            }
        }
    }

    // Player death check
    if (_wizard.isDead() && _state == GameState::Playing) {
        _state = GameState::GameOver;
        printf("Game Over!\n");
    }

    // Apply terrain height
    _wizard.transform.position.y = _terrain.heightAt(
        _wizard.transform.position.x, _wizard.transform.position.z);
    for (auto& m : _mobs) {
        m->transform.position.y = _terrain.heightAt(
            m->transform.position.x, m->transform.position.z);
    }

    // Update projectiles
    _wizard.update(dt);
    for (auto& p : _projectiles) p->update(dt);
    _damageNumbers.update(dt);

    // Cleanup
    _projectiles.erase(
        std::remove_if(_projectiles.begin(), _projectiles.end(),
            [](auto& p) { return p->destroyed(); }),
        _projectiles.end());
    _mobs.erase(
        std::remove_if(_mobs.begin(), _mobs.end(),
            [](auto& m) { return m->destroyed(); }),
        _mobs.end());
}

void Game::render() {
    _renderer.clear();
    _renderer.begin(_camera);

    _renderer.drawMesh(_terrain.mesh(), _whiteTex, glm::mat4(1));

    // Draw trees
    for (auto& t : _forest.trees()) {
        glm::vec3 pos = t.position;
        pos.y = _terrain.heightAt(pos.x, pos.z);
        _renderer.drawMesh(_cube, _whiteTex,
            glm::translate(glm::mat4(1), pos + glm::vec3(0, t.scale, 0))
            * glm::rotate(glm::mat4(1), t.rotation, glm::vec3(0,1,0))
            * glm::scale(glm::mat4(1), glm::vec3(0.3f, t.scale * 2, 0.3f)));
    }

    // Draw rocks
    for (auto& r : _forest.rocks()) {
        glm::vec3 pos = r.position;
        pos.y = _terrain.heightAt(pos.x, pos.z);
        _renderer.drawMesh(_cube, _whiteTex,
            glm::translate(glm::mat4(1), pos)
            * glm::scale(glm::mat4(1), glm::vec3(r.scale * 0.5f,
                r.scale * 0.3f, r.scale * 0.5f)));
    }

    // Draw dragon
    if (_bossSpawned && !_bossDefeated && _dragon.health.alive()) {
        _renderer.drawMesh(_cube, _whiteTex,
            glm::translate(glm::mat4(1), _dragon.transform.position)
            * glm::rotate(glm::mat4(1), _dragon.transform.rotation,
                glm::vec3(0, 1, 0))
            * glm::scale(glm::mat4(1), _dragon.transform.scale));
    }

    // Draw wizard
    _renderer.drawMesh(_cube, _whiteTex,
        glm::translate(glm::mat4(1), _wizard.transform.position));

    // Draw mobs
    for (auto& m : _mobs) {
        if (!m->destroyed()) {
            _renderer.drawMesh(_cube, _whiteTex,
                glm::translate(glm::mat4(1), m->transform.position)
                * glm::scale(glm::mat4(1), m->transform.scale));
        }
    }

    // Draw projectiles
    for (auto& p : _projectiles) {
        if (!p->destroyed()) {
            _renderer.drawMesh(_cube, _whiteTex,
                glm::translate(glm::mat4(1), p->transform.position));
        }
    }

    _damageNumbers.render(_camera.view(), _camera.proj());

    _renderer.end();

    // HUD
    float bossHpRatio = _bossSpawned && !_bossDefeated
        ? (float)_dragon.health.current / _dragon.health.max : 0;
    _hud.render(_wizard, _spellBook, _spawner.killCount(),
        _bossSpawned && !_bossDefeated, bossHpRatio);
}

void Game::processInput() {
}
