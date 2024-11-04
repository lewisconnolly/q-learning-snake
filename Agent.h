#include "Global.h"
#include "AdaptiveLoopDetector.h"

class Agent
{
public:    
    

    enum Action { LEFT = 0, RIGHT, STRAIGHT};

    Agent();
    void Reset();
    Action Act(std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir);
    std::map<std::string, std::array<float, ACTION_NUM>> LoadLM(std::string path);
    void SaveLM(std::string path);
    struct global::GameState GetState(std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir);
    std::string GetStateStr(global::GameState gameState);
    void UpdateLM(std::string* deathReason, std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir);

    float epsilon;
    float learningRate;
    float discountRate;    
    std::map<std::string, std::array<float, ACTION_NUM>> LM;
    AdaptiveLoopDetector detector;
    int numLoops;
};