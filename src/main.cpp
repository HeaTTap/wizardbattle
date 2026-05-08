#include "core/Game.h"
#include <SDL3/SDL.h>
#include <unistd.h>

int main() {
    // Set working directory to project root so asset paths resolve correctly
    const char* base = SDL_GetBasePath();
    if (base) {
        chdir(base);
        chdir("..");  // Go from build/ to project root
    }
    Game game;
    game.run();
    return 0;
}
