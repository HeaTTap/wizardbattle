#include "player/Progression.h"
#include <cmath>
#include <glm/glm.hpp>

int Progression::xpForLevel(int level) {
    return (int)(100.0 * powf(1.5f, level - 1));
}

int Progression::xpForNextLevel(int level) {
    return xpForLevel(level + 1);
}

float Progression::xpProgress(int xp, int level) {
    if (level >= 10) return 1.0f;
    int needed = xpForLevel(level);
    return glm::clamp((float)xp / (float)needed, 0.0f, 1.0f);
}
