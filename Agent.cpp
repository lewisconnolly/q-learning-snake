#include "Agent.h"

namespace nh = nlohmann;

Agent::Agent()
{
    // Q-learning equation parameters
    epsilon = 1.0;
    learningRate = 0.75;
    discountRate = 0.25;
    numLoops = 0;

    // Learning matrix and action history
    LM = LoadLM("lm.json");
    detector = AdaptiveLoopDetector::AdaptiveLoopDetector();
}

void Agent::Reset()
{
    // Get number of repetitions of largest loop
    AdaptiveLoopDetector::LoopInfo loopInfo = detector.detectLoop();
    if (loopInfo.found)
    {
        numLoops = loopInfo.repetitions;
    }
    detector.clearHistory();
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
    
    global::GameState state = GetState(snakePartsList, foodPos, snakeDir);
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
    global::HistoryEntry newHistoryEntry;
    newHistoryEntry.actionKey = actionKey;
    newHistoryEntry.state = state;
    detector.addState(newHistoryEntry);

    return action;
}

struct global::GameState Agent::GetState(std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir)
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
    std::string tailSidesStr = "0000";
    bool foundLeft = false;
    bool foundRight = false;
    bool foundAbove = false;
    bool foundBelow = false;
    for (Vec2 t : snakeTail)
    {
        if (!foundLeft && t.y == snakeHead.y && t.x < snakeHead.x) // Tail part to left of head
        {            
            surroundingsStr[0] = '1';
            foundLeft = true;
        }
        else if (!foundRight && t.y == snakeHead.y && t.x > snakeHead.x) // Tail part to right of snake
        {            
            surroundingsStr[1] = '1';
            foundRight = true;
        }
        else if (!foundAbove && t.y < snakeHead.y && t.x == snakeHead.x) // Tail part above of snake
        {                        
            surroundingsStr[2] = '1';
            foundAbove = true;
        }
        else if (!foundBelow && t.y > snakeHead.y && t.x == snakeHead.x) // Tail part below of snake
        {            
            surroundingsStr[3] = '1';
            foundBelow = true;
        }
    }

    // Create and return new game state
    global::GameState newGameState;
    newGameState.distToFood = Vec2(foodDistX, foodDistY);
    newGameState.foodDirX = foodDirX;
    newGameState.foodDirY = foodDirY;
    newGameState.foodPos = *foodPos;
    newGameState.surroundings = surroundingsStr;
    newGameState.tailSides = tailSidesStr;
    newGameState.snakeDir = std::to_string(*snakeDir);
        
    return newGameState;
}

std::string Agent::GetStateStr(global::GameState gameState)
{
    std::string gameStateStr = "(" + gameState.foodDirX + "," + gameState.foodDirY + "," + gameState.snakeDir + "," + gameState.tailSides + "," + gameState.surroundings + ")";
    return gameStateStr;
}

void Agent::UpdateLM(std::string* deathReason, std::vector<Vec2>* snakePartsList, Vec2* foodPos, int* snakeDir)
{
    bool gotFood = false;
    float reward = 0.0; // Intialise reward    
    float currStateMaxReward = 0.0; // Intialise maximum reward in current state (pre-action). Stays 0 if agent died
 
    global::HistoryEntry prevEntry = detector.getLastEntry();
    global::GameState prevState = prevEntry.state; // Pre last action state    
    std::string prevStateStr = GetStateStr(prevState); // Get state string for last state
    int prevActionKey = prevEntry.actionKey; // Last action
    global::GameState currState = GetState(snakePartsList, foodPos, snakeDir); // State now (after effect applied from action)

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

        if (currState.foodPos != prevState.foodPos) // If food position changed between states then it was eaten and so reward is extra large positive
        {
            reward = 2.0 * rewardModifier;
            // Boost learning rate slightly when food is collected
            learningRate = std::min(1.0, learningRate * 1.025);
            gotFood = true;
        }
        else if (fabs(currDistToFood.x) < fabs(prevDistToFood.x) || fabs(currDistToFood.y) < fabs(prevDistToFood.y)) // If snake moved closer to food after action in prev state -> reward is medium positive
        {
            reward = 1.0 * rewardModifier;
        }
        else if (fabs(currDistToFood.x) != fabs(prevDistToFood.x) || fabs(currDistToFood.y) < fabs(prevDistToFood.y)) // Snake moved farther away from food -> reward is medium negative 
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
        reward = -2.0 * rewardModifier;
    }

    // Punish actions that led to loops when no food was collected
    if (!gotFood)
    {
        AdaptiveLoopDetector::LoopInfo loopInfo = detector.detectLoop();
        if (loopInfo.found)
        {
            reward -= 0.5;
        }
    }

    // Update previous state Q-value based on reward and potential future rewards (discounted)
    LM[prevStateStr][prevActionKey] = LM[prevStateStr][prevActionKey] + learningRate * (reward + discountRate * currStateMaxReward - LM[prevStateStr][prevActionKey]);
}
