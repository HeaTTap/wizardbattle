#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <unordered_map>

class Input {
public:
    void update();

    bool keyDown(SDL_Keycode key) const { return _keys.count(key) && _keys.at(key); }
    bool keyPressed(SDL_Keycode key) const { return _justPressed.count(key); }
    bool mouseDown(int button) const { return _mouseButtons & SDL_BUTTON_MASK(button); }
    bool mousePressed(int button) const { return _justClicked.count(button); }
    int mouseX() const { return _mx; }
    int mouseY() const { return _my; }
    int mouseWheel() const { return _wheel; }

private:
    std::unordered_map<SDL_Keycode, bool> _keys;
    std::unordered_map<SDL_Keycode, bool> _justPressed;
    std::unordered_map<int, bool> _justClicked;
    int _mx = 0, _my = 0, _wheel = 0;
    Uint32 _mouseButtons = 0;
};
