#pragma once
#include "core/Window.h"
#include "core/Renderer.h"
#include "core/Camera.h"
#include "core/Input.h"
#include "world/Terrain.h"
#include "world/ForestMap.h"
#include "entity/GameObject.h"
#include "entity/Collision.h"
#include "player/Wizard.h"
#include "player/Spell.h"
#include "player/SpellBook.h"
#include "mob/Spawner.h"
#include "mob/Goblin.h"
#include "mob/Archer.h"
#include "mob/Ogre.h"
#include "mob/Dragon.h"
#include "ui/HUD.h"
#include "ui/DamageNumbers.h"
#include <vector>
#include <memory>

enum class GameState {
    MainMenu,
    Playing,
    GameOver,
    Victory,
};

class Game {
public:
    Game();
    void run();

private:
    void update(float dt);
    void render();
    void processInput();

    Window _window;
    Renderer _renderer;
    Camera _camera;
    Input _input;
    Terrain _terrain;
    ForestMap _forest;
    GameState _state = GameState::Playing;
    std::vector<std::unique_ptr<GameObject>> _objects;

    Mesh _cube;
    Texture _whiteTex;

    Wizard _wizard;
    SpellBook _spellBook;
    std::vector<std::unique_ptr<GameObject>> _projectiles;

    Spawner _spawner{200.0f};
    std::vector<std::unique_ptr<GameObject>> _mobs;

    Dragon _dragon;
    bool _bossSpawned = false;
    bool _bossDefeated = false;

    HUD _hud;
    DamageNumbers _damageNumbers;
};
