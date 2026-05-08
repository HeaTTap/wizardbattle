#include "core/Game.h"
#include <SDL3/SDL.h>
#include <unistd.h>
#include <cstdlib>

int main() {
    // Force X11 backend on Wayland for GLEW compatibility
    setenv("SDL_VIDEO_DRIVER", "x11", 0);

    // Set working directory to project root so asset paths resolve correctly
    const char* base = SDL_GetBasePath();
    if (base) {
        chdir(base);
        chdir("..");
    }
    Game game;
    game.run();
    return 0;
}
