# WizardBattle — 3D PvE Wizard Game

**Date:** 2026-05-08
**Type:** Game
**Language:** C++ (GCC 16.1.1)
**Libraries:** SDL3, OpenGL 3.3+, OpenAL, GLM (header-only)

## Overview

Top-down 3D PvE action game. Player is a wizard in a forest, killing mobs (goblins, archers, ogres) to gain XP and level up. At level 1/3/5/8/10 the wizard unlocks new spells. After 100 kills, a Forest Dragon boss spawns in a central arena. Victory on boss kill; game continues after.

## Architecture

Lightweight composition pattern — GameObject owns pluggable components (Transform, Health, Renderable, Collider, AI). Inheritance only where natural (all spells share a base Spell class, all mob types share a MobAI component).

### Sub-projects (build order)

1. **Core Engine** — window, game loop, rendering pipeline, camera, input, asset loading, terrain
2. **Player & Combat** — wizard movement, basic projectile, damage, health, XP
3. **Mobs & AI** — goblin, archer, ogre with state-machine AI, spawning, kill tracking
4. **Spells & Progression** — 5 spells, mana, cooldowns, leveling, spell unlock UI
5. **Boss Fight** — dragon, phases, arena, victory screen
6. **Polish** — particles, sound, HUD, death/victory screens, menu

## Project Structure

```
~/Documents/wizardbattle/
├── CMakeLists.txt
├── assets/
│   ├── models/      (.obj/.gltf)
│   ├── textures/
│   ├── sounds/
│   └── shaders/     (.vert/.frag GLSL)
├── src/
│   ├── main.cpp
│   ├── core/        (Game, Window, Renderer, Input, AssetManager, Camera)
│   ├── entity/      (GameObject, Components, Collision)
│   ├── player/      (Wizard, Spell, SpellBook, Progression)
│   ├── mob/         (MobAI, Goblin, Archer, Ogre, Dragon, Spawner)
│   ├── world/       (Terrain, ForestMap)
│   └── ui/          (HUD, HealthBar, ManaBar, XPBar, SpellBar, KillCounter)
└── docs/
```

## Core Engine

- Fixed timestep game loop at 60 FPS
- SDL3 for window (1280×720 default), input, OpenGL context, audio
- OpenGL 3.3+ Core profile
- Top-down orthographic camera, angled ~60 degrees from horizontal
- Single directional light (sun) + ambient + fog
- Terrain: flat grid with noise-based height variation, rendered as indexed mesh
- Skybox: gradient clear color (sky blue to horizon)
- Camera follows wizard with smooth lerp, mouse wheel zoom
- AABB collision with spatial grid broad-phase
- WASD move, mouse aim, 1-5 or scroll to select spell, left-click to cast

## Player & Wizard

### Stats
- Health: 100 + 10/level
- Mana: 100 + 15/level (regens 5/sec)
- Move speed: 5 units/sec
- XP to level: starts 100, scales x1.5 per level, max level 10

## Spells

| # | Spell | Unlock | Mana | Damage | Cooldown | Behavior |
|---|-------|--------|------|--------|----------|----------|
| 1 | Firebolt | L1 | 15 | 25 | 0.8s | Fast projectile, small AOE on impact |
| 2 | Ice Shard | L3 | 25 | 20 | 1.5s | Pierces enemies, 40% slow 2s |
| 3 | Lightning Strike | L5 | 35 | 45 | 3.0s | Instant chain, hits 3 nearby targets |
| 4 | Arcane Nova | L8 | 50 | 30 AOE | 6.0s | Point-blank area-of-effect burst around wizard |
| 5 | Meteor | L10 | 80 | 100 AOE | 12.0s | Targeted delayed impact, large radius |

Cooldowns are per-spell, not global. Mana is shared. All spells remain viable at max level.

## Progression

- Goblin: 30 XP, Archer: 45 XP, Ogre: 70 XP, Dragon: 500 XP
- Level up: on-screen notification, new spell added to bar
- Level 10 is cap

## Mobs & AI

### Types

| Mob | HP | Damage | Speed | Detection | Behavior |
|-----|-----|--------|-------|-----------|----------|
| Goblin | 40 | 8 | Fast | 12 units | Swarms, erratic movement, flees <20% HP |
| Forest Archer | 30 | 12 | Slow | 15 units | Keeps distance, fires arrows, repositions |
| Ogre | 100 | 20 | Very slow | 10 units | Charges straight, heavy telegraph |

### AI State Machine

All mobs share: Idle → Aggro → (Chase | Ranged | Flee) → Dead

- **Idle:** wander randomly within spawn zone
- **Aggro:** triggered on player proximity (detection radius varies)
- **Chase:** A* pathfinding on spatial grid, attack on contact
- **Ranged:** maintain 8-12 unit distance, fire every 2s, reposition if too close
- **Flee:** goblin-only, run from player when <20% HP
- **Dead:** death animation, grant XP, despawn after 3s

### Spawning

- 15-20 mobs alive at any time
- Respawn timer: 5-10 seconds after death, at map edge
- Distribution: ~50% goblin, 30% archer, 20% ogre

## Boss — Forest Dragon

Spawns at map center clearing when kill count ≥ 100. Arena fog walls appear during fight.

- **HP:** 2000, **Damage:** 35 (swipe), 50 (fire breath), 25 (tail)
- **Size:** 3x larger than ogre model

### Phases

| Phase | HP Range | Attacks |
|-------|----------|---------|
| 1 | 100-60% | Melee swipe (front arc), tail swipe (rear arc) |
| 2 | 60-30% | Adds fire breath (cone AOE, telegraphed by head rearing) |
| 3 | 30-0% | Enrage — faster attacks, breath becomes sweeping arc |

On death: 500 XP, victory screen, continue playing.

## World / Forest Map

- 200×200 unit playable area
- Heightmap terrain with gentle hills via Perlin noise
- Scattered trees (impassable obstacles) and rocks
- Central clearing (40×40) for boss fight — no trees, flat ground
- Map edges: invisible walls or dense tree line
- Mini-map in HUD corner

## UI / HUD

- Top-left: health bar (red), mana bar (blue)
- Bottom: spell bar (5 slots, locked spells greyed out)
- Bottom-center: XP bar with level indicator
- Top-right: kill counter ("Kills: 42 / 100")
- Center: damage numbers (floating text on hit)
- Boss fight: boss health bar at top-center
- On death: "You Died" overlay with respawn button
- On boss kill: "Victory!" overlay
- Main menu: "Play" / "Quit"

## Assets Plan (< 1GB total)

Sources (free/CCO-licensed):
- Models: wizard, goblin, ogre, archer, dragon, trees, rocks (from OpenGameArt, Sketchfab CC0, Quaternius)
- Textures: forest floor, grass, bark, rock, sky, spell VFX
- Sounds: spell casts, hits, mob growls, boss roar, ambient forest, BGM (from Freesound, OpenGameArt)
- Format: .obj for models, PNG for textures, WAV/OGG for audio

## Build

```bash
cd ~/Documents/wizardbattle
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./wizardbattle
```

### Dependencies (Arch/CachyOS)
```
sdl3 opengl glu glew glm openal
```
