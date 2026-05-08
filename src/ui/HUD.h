#pragma once
#include <string>

class Wizard;
class SpellBook;
class Shader;
class Mesh;

class HUD {
public:
    HUD();
    void render(const Wizard& wizard, const SpellBook& spells,
        int killCount, bool bossActive, float bossHpRatio);

private:
    void drawBar(float x, float y, float w, float h,
        float fill, float r, float g, float b, float bgR, float bgG, float bgB);
    void drawText(const std::string& text, float x, float y, float scale);

    Shader* _shader = nullptr;
    Mesh* _quad = nullptr;
    Mesh* _barMesh = nullptr;
};
