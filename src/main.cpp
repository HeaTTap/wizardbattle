#include "core/Window.h"
#include <cstdio>

int main() {
    Window window("WizardBattle", 1280, 720);
    if (window.shouldClose()) return 1;

    while (!window.shouldClose()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                window.setShouldClose(true);
            }
        }
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        window.swap();
    }
    return 0;
}
