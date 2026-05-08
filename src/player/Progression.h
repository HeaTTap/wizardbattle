#pragma once

class Progression {
public:
    static int xpForLevel(int level);
    static float xpProgress(int xp, int level);
    static int xpForNextLevel(int level);
};
