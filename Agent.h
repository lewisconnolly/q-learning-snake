#include "Vec2.h"
#include <string>
#include <vector>
#include <map>
#include <array>
#include "Global.h"

class Agent
{
public:    
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
    };

    enum Action { LEFT = 0, RIGHT, STRAIGHT};

    Agent();
    void Reset();
    Action Act(std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir);
    std::map<std::string, std::array<float, ACTION_NUM>> LoadLM(std::string path);
    void SaveLM(std::string path);
    struct GameState GetState(std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir);
    std::string GetStateStr(GameState gameState);    
    void UpdateLM(std::string* deathReason, std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir);

    float epsilon;
    float learningRate;
    float discountRate;    
    std::vector<HistoryEntry> history;
    std::map<std::string, std::array<float, ACTION_NUM>> LM;
};