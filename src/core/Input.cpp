#include "core/Input.h"

void Input::update() {
    _justPressed.clear();
    _justClicked.clear();
    _wheel = 0;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_EVENT_KEY_DOWN:
            _justPressed[e.key.key] = !e.key.repeat;
            _keys[e.key.key] = true;
            break;
        case SDL_EVENT_KEY_UP:
            _keys[e.key.key] = false;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            _mx = (int)e.motion.x;
            _my = (int)e.motion.y;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            _mouseButtons |= SDL_BUTTON_MASK(e.button.button);
            _justClicked[e.button.button] = true;
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            _mouseButtons &= ~SDL_BUTTON_MASK(e.button.button);
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            _wheel = (int)e.wheel.y;
            break;
        }
    }
}
