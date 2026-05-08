#include "ui/HUD.h"
#include "core/Shader.h"
#include "core/Mesh.h"
#include "player/Wizard.h"
#include "player/SpellBook.h"
#include "player/Progression.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>

HUD::HUD() {
    _shader = new Shader("assets/shaders/default.vert", "assets/shaders/default.frag");
    _quad = new Mesh(Mesh::createQuad());
    _barMesh = new Mesh(Mesh::createQuad());
}

void HUD::render(const Wizard& wizard, const SpellBook& spells,
    int killCount, bool bossActive, float bossHpRatio) {

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _shader->use();
    glm::mat4 ortho = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f);
    _shader->setMat4("uProj", &ortho[0][0]);
    glm::mat4 identity(1);
    _shader->setMat4("uView", &identity[0][0]);
    _shader->setMat4("uModel", &identity[0][0]);
    _shader->setVec3("uLightDir", 0, -1, 0);
    _shader->setVec3("uLightColor", 1, 1, 1);
    _shader->setVec3("uAmbient", 1, 1, 1);
    _shader->setVec3("uViewPos", 0, 0, 0);
    _shader->setVec3("uFogColor", 0, 0, 0);
    _shader->setFloat("uFogStart", 9999);
    _shader->setFloat("uFogEnd", 9999);

    auto drawRect = [&](float x, float y, float w, float h, float r, float g, float b, float a) {
        _shader->setVec3("uLightColor", r, g, b);
        _shader->setVec3("uAmbient", r, g, b);
        glm::mat4 m = glm::translate(identity, glm::vec3(x, y, 0))
            * glm::scale(identity, glm::vec3(w, h, 1));
        _shader->setMat4("uModel", &m[0][0]);
        _shader->setInt("uTexture", -1);
        _quad->draw();
    };

    float hpPct = (float)wizard.health.current / wizard.health.max;
    drawRect(20, 20, 200, 22, 0.15f, 0.15f, 0.15f, 0.8f);
    drawRect(20, 20, 200 * hpPct, 22, 0.9f, 0.15f, 0.15f, 1.0f);

    float mpPct = wizard.mana / wizard.maxMana;
    drawRect(20, 46, 200, 18, 0.15f, 0.15f, 0.15f, 0.8f);
    drawRect(20, 46, 200 * mpPct, 18, 0.15f, 0.4f, 0.9f, 1.0f);

    float xpPct = Progression::xpProgress(wizard.xp -
        (wizard.level > 1 ? Progression::xpForLevel(wizard.level) : 0),
        wizard.level > 1 ? wizard.level : 1);
    if (wizard.level >= 10) xpPct = 1.0f;
    drawRect(440, 690, 400, 14, 0.1f, 0.1f, 0.1f, 0.8f);
    drawRect(440, 690, 400 * xpPct, 14, 0.9f, 0.8f, 0.2f, 1.0f);

    for (int i = 0; i < 5; i++) {
        float sx = 390 + i * 100;
        bool unlocked = spells.cooldownRemaining(i) >= 0;
        float cd = spells.cooldownRemaining(i);

        float r = unlocked ? 0.3f : 0.15f;
        float g = unlocked ? 0.3f : 0.15f;
        float b = unlocked ? 0.3f : 0.15f;
        drawRect(sx, 655, 90, 30, r, g, b, 0.9f);

        if (cd > 0 && unlocked) {
            drawRect(sx, 655, 90, 30, 0.1f, 0.1f, 0.1f, 0.6f);
        }
    }

    char killText[64];
    snprintf(killText, sizeof(killText), "Kills: %d / 100", killCount);

    if (bossActive) {
        drawRect(390, 10, 500, 20, 0.1f, 0.1f, 0.1f, 0.8f);
        drawRect(390, 10, 500 * bossHpRatio, 20, 0.9f, 0.2f, 0.2f, 1.0f);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HUD::drawBar(float x, float y, float w, float h,
    float fill, float r, float g, float b, float bgR, float bgG, float bgB) {}

void HUD::drawText(const std::string& text, float x, float y, float scale) {}
