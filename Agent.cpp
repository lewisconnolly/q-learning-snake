#include "Agent.h"
#include <fstream>
#include "Global.h"
#include <algorithm>
#include <math.h> 
#include <string>
#include "json.hpp"

namespace nh = nlohmann;

Agent::Agent()
{
    // Q-learning equation parameters
    epsilon = 0.1;
    learningRate = 0.8;
    discountRate = 0.2;

    // Learning matrix and action history
    LM = LoadLM("lm.json");
    history = {};    
}

void Agent::Reset()
{
    history = {};
}

std::map<std::string, std::array<float, ACTION_NUM>> Agent::LoadLM(std::string path)
{
    // Create file object
    std::ifstream f(path);
    // Parse file as JSON
    nh::json data = nh::json::parse(f);
    // Convert JSON to learning matrix
    data.get_to(LM);

    return LM;
}

void Agent::SaveLM(std::string path)
{
    // Convert learning matrix to JSON
    nh::json data = LM;
    // Create file object
    std::ofstream f(path);
    // Write to file
    f << data;
}

Agent::Action Agent::Act(std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir)
{
    // Initialize random seed
    srand(time(NULL));
    
    GameState state = GetState(snakePartsList, foodPos, snakeDir);
    int actionKey = 0;

    // Choose a random action (epsilon * 100)% of the time
    int random = rand() % 100;
    if (random < (epsilon * 100.0))
    {
        // Random action key
        actionKey = rand() % ACTION_NUM;
    }
    else
    {
        // Get rewards per action for current state from learning matrix
        std::array<float, ACTION_NUM> stateRewards = LM[GetStateStr(state)];

        // Get highest reward
        float maxReward = stateRewards[0];
        for (int i = 1; i < ACTION_NUM; ++i)
        {
            if (stateRewards[i] > maxReward)
            {
                maxReward = stateRewards[i];
            }
        }

        // Get index of highest reward        
        for (int i = 0; i < ACTION_NUM; ++i)
        {
            if (stateRewards[i] == maxReward)
            {
                actionKey = i;
                break;
            }
        }
    }
    
    // Get action from key
    Action action = Action(actionKey);

    // Add state and action to history list
    HistoryEntry newHistoryEntry;
    newHistoryEntry.actionKey = actionKey;
    newHistoryEntry.state = state;
    history.push_back(newHistoryEntry);

    return action;
}

struct Agent::GameState Agent::GetState(std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir)
{
    // Get coordinates of snake's head and distance to food
    Vec2 snakeHead = snakePartsList->back();
    float foodDistX = foodPos->x - snakeHead.x;
    float foodDistY = foodPos->y - snakeHead.y;
    std::string foodDirX = "";
    std::string foodDirY = "";

    // Define horizontal direction (left, right or in line) to food 
    if (foodDistX > 0) // Food is to right
    {
        foodDirX = "1";
    }
    else if (foodDistX < 0) // Food is to left
    {
        foodDirX = "0";
    }
    else // Food is on same horizontal row
    {
        foodDirX = "NA";
    }

    // Define vertical direction (left, right or in line) to food 
    if (foodDistY > 0) // Food is below
    {
        foodDirY = "3";
    }
    else if (foodDistX < 0) // Food is above
    {
        foodDirY = "2";
    }
    else // Food is on same vertical row
    {
        foodDirY = "NA";
    }

    // Get the coordinates of 4 squares surrounding snake's head
    std::array<Vec2, 4> surroundingSqs = {
        Vec2(snakeHead.x - global::BLOCK_SIZE, snakeHead.y), // Left of snake's head
        Vec2(snakeHead.x + global::BLOCK_SIZE, snakeHead.y), // Right of snake's head
        Vec2(snakeHead.x, snakeHead.y - global::BLOCK_SIZE), // Above snake's head
        Vec2(snakeHead.x, snakeHead.y + global::BLOCK_SIZE) // Below snake's head
    };    

    // Define string representing squares surrounding snake's head
    std::string surroundingsStr = "";
    std::vector<Vec2> snakeTail(snakePartsList->begin(), snakePartsList->end() - 1);

    for (Vec2 sq : surroundingSqs)
    {
        if (sq.x < 0 || sq.y < 0) // Square is off the screen to the left or top
        {
            surroundingsStr += "1";
        }
        else if (sq.x >= global::WINDOW_WIDTH || sq.y >= global::WINDOW_HEIGHT) // Square is off the screen to the right or bottom
        {
            surroundingsStr += "1";
        }
        else if (std::find(snakeTail.begin(), snakeTail.end(), sq) != snakeTail.end()) // Square is part of snake's tail
        {
            surroundingsStr += "1";
        }
        else
        {
            surroundingsStr += "0";
        }
    }

    // Calculate number of sides tail is on
    int numTailSides = 0;
    bool foundLeft = false;
    bool foundRight = false;
    bool foundAbove = false;
    bool foundBelow = false;
    for (Vec2 t : snakeTail)
    {
        if (!foundLeft && t.y == snakeHead.y && t.x < snakeHead.x) // Tail part to left of head
        {            
            numTailSides++;
        }
        else if (!foundRight && t.y == snakeHead.y && t.x > snakeHead.x) // Tail part to right of snake
        {            
            numTailSides++;
        }
        else if (!foundAbove && t.y < snakeHead.y && t.x == snakeHead.x) // Tail part above of snake
        {                        
            numTailSides++;
        }
        else if (!foundBelow && t.y > snakeHead.y && t.x == snakeHead.x) // Tail part below of snake
        {            
            numTailSides++;
        }
    }

    // Create and return new game state
    GameState newGameState;
    newGameState.distToFood = Vec2(foodDistX, foodDistY);
    newGameState.foodDirX = foodDirX;
    newGameState.foodDirY = foodDirY;
    newGameState.foodPos = *foodPos;
    newGameState.surroundings = surroundingsStr;
    newGameState.tailSides = std::to_string(numTailSides);
    newGameState.snakeDir = std::to_string(*snakeDir);
        
    return newGameState;
}

std::string Agent::GetStateStr(Agent::GameState gameState)
{
    std::string gameStateStr = "(" + gameState.foodDirX + "," + gameState.foodDirY + "," + gameState.snakeDir + "," + gameState.tailSides + "," + gameState.surroundings + ")";
    return gameStateStr;
}

void Agent::UpdateLM(std::string* deathReason, std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir)
{
    float reward = 0.0; // Intialise reward    
    float currStateMaxReward = 0.0; // Intialise maximum reward in current state (pre-action). Stays 0 if agent died

    GameState prevState = history.back().state; // Pre last action state    
    std::string prevStateStr = GetStateStr(prevState); // Get state string for last state
    int prevActionKey = history.back().actionKey; // Last action
    GameState currState = GetState(snakePartsList, foodPos, snakeDir); // State now (after effect applied from action)

    // If the action was straight then increase or decrease the reward
    // Encourage moving towards food if already on track and discourage continuing to move away if off track
    float rewardModifier = 1.0;
    if (Action(prevActionKey) == Action::STRAIGHT)
    {
        rewardModifier = 2.0;
    }

    if (*deathReason == "")
    {
        // Distance to food for each state
        Vec2 currDistToFood = currState.distToFood;
        Vec2 prevDistToFood = prevState.distToFood;        

        if (currState.foodPos != prevState.foodPos) // If food position changed between states then it was eaten and so reward is positive
        {
            reward = 1.5 * rewardModifier;
        }
        else if (fabs(currDistToFood.x) < fabs(prevDistToFood.x) || fabs(currDistToFood.y) < fabs(prevDistToFood.y)) // If snake moved closer to food after action in prev state -> reward is positive
        {
            reward = 1.0 * rewardModifier;
        }
        else // Snake moved farther away from food -> reward is negative
        {
            reward = -1.0 * rewardModifier;
        }
                
        // Get state string for current state
        std::string currStateStr = GetStateStr(currState);

        // Get rewards per action for current state from learning matrix
        std::array<float, ACTION_NUM> stateRewards = LM[currStateStr];

        // Get highest reward for current state
        currStateMaxReward = stateRewards[0];
        for (int i = 1; i < ACTION_NUM; ++i)
        {
            if (stateRewards[i] > currStateMaxReward)
            {
                currStateMaxReward = stateRewards[i];
            }
        }        
    }
    else
    {
        reward = -1.5 * rewardModifier; // Negative reward if action led to death
    }

    // Update previous state Q-value based on reward and potential future rewards (discounted)
    LM[prevStateStr][prevActionKey] = LM[prevStateStr][prevActionKey] + learningRate * (reward + discountRate * currStateMaxReward - LM[prevStateStr][prevActionKey]);
}
