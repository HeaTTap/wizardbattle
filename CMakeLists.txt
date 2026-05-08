cmake_minimum_required(VERSION 3.20)
project(wizardbattle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)
pkg_check_modules(GLEW REQUIRED glew)
pkg_check_modules(OPENAL REQUIRED openal)

# GLM is header-only, bundle it or find it
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "GLM not found. Install with: sudo pacman -S glm")
endif()

add_executable(wizardbattle
    src/main.cpp
    src/core/Window.cpp
    src/core/Renderer.cpp
    src/core/Camera.cpp
    src/core/Input.cpp
    src/core/Game.cpp
    src/core/Shader.cpp
    src/core/Mesh.cpp
    src/core/Texture.cpp
    src/entity/GameObject.cpp
    src/entity/Components.cpp
    src/entity/Collision.cpp
    src/player/Wizard.cpp
    src/player/Spell.cpp
    src/player/SpellBook.cpp
    src/player/Progression.cpp
    src/mob/MobAI.cpp
    src/mob/Goblin.cpp
    src/mob/Archer.cpp
    src/mob/Ogre.cpp
    src/mob/Spawner.cpp
    src/mob/Dragon.cpp
    src/world/Terrain.cpp
    src/world/ForestMap.cpp
    src/ui/HUD.cpp
    src/ui/DamageNumbers.cpp
)

target_include_directories(wizardbattle PRIVATE
    src
    ${SDL3_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
)

target_link_libraries(wizardbattle
    ${SDL3_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENAL_LIBRARIES}
)

target_compile_options(wizardbattle PRIVATE -Wall -Wextra)
