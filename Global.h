#include "Vec2.h"
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <math.h> 
#include <array>
#include <map>
#include "json.hpp"

#ifndef _GLOBAL_ 

#define _GLOBAL_ 


namespace global
{
    const int WINDOW_WIDTH = 640;
    const int WINDOW_HEIGHT = 480;
    const int BLOCK_SIZE = 10;
    const int FRAME_DELAY_MS = 0;
    const int LM_SAVE_INTERVAL = 100;
    const int GAMES_LIMIT = 100000;
    const float EPSILON_DECAY = 0.997;
    const float EPSILON_MIN = 0.01;

    struct GameState
    {
        Vec2        distToFood;
        std::string foodDirX;
        std::string foodDirY;
        std::string surroundings;
        Vec2        foodPos;
        std::string snakeDir;
        std::string tailSides;
    };

    struct HistoryEntry
    {
        GameState   state;
        int         actionKey;

        bool operator==(const HistoryEntry& other) const {
            return actionKey == other.actionKey &&
                state.foodDirX == other.state.foodDirX &&
                state.foodDirY == other.state.foodDirY &&
                state.snakeDir == other.state.snakeDir &&
                state.tailSides == other.state.tailSides &&
                state.surroundings == other.state.surroundings;
        }
    };
}

#endif // _GLOBAL_

#ifndef _ACTION_NUM_ 
#define ACTION_NUM 3
#endif // _ACTION_NUM_