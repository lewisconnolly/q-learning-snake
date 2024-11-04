#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "Global.h"
#include <stdio.h>
#include <chrono>
#include <utility>
#include "Agent.h"
#include <numeric>
#include <sstream>
//#include "matplotlibcpp.h"

namespace nh = nlohmann;
//namespace plt = matplotlibcpp;

bool running = true;
std::vector<int>scores = {};

struct GameResult
{
	int			score = 0;
	std::string deathReason = "";
};

enum Direction { WEST = 0, EAST, NORTH, SOUTH };

void InitLM()
{
	std::array<std::string, 16> surroundings = { "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111" };
	std::array<std::string, 4> snakeDir = { "0", "1", "2", "3"};
	std::array<std::string, 16> tailSides = { "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111" };
	std::array<std::string, 3> foodDirX = { "0", "1", "NA" };
	std::array<std::string, 3> foodDirY = { "2", "3", "NA" };	
	
	std::map<std::string, std::array<float, ACTION_NUM>> LM;
	std::string stateString = "";
	for (const std::string& i : foodDirX)
	{
		for (const std::string& j : foodDirY)
		{
			for (const std::string& k : snakeDir)
			{
				for (const std::string& l : tailSides)
				{
					for (const std::string& m : surroundings)
					{
						stateString = "(" + i + "," + j + "," + k + "," + l + "," + m + ")";
						LM[stateString] = { 0.0, 0.0, 0.0 };
					}
				}
			}
		}
	}

	// Convert learning matrix to JSON
	nh::json data = LM;
	// Create file object
	std::ofstream f("lm.json");
	// Write to file
	f << data;
}

void DrawFood(Vec2* foodPos, SDL_Renderer* renderer)
{
	SDL_Rect rect{};
	
	// Set size and position of food
	rect.w = global::BLOCK_SIZE;
	rect.h = global::BLOCK_SIZE;
	rect.x = static_cast<int>(foodPos->x);
	rect.y = static_cast<int>(foodPos->y);

	// Set draw colour to green and fill rectangle
	SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 0xFF);
	SDL_RenderFillRect(renderer, &rect);
}

void DrawSnake(std::vector<Vec2>* snakePartsList, SDL_Renderer* renderer)
{
	int i = 0;
	int size = snakePartsList->size();
	for (Vec2& p : *snakePartsList)
	{
		SDL_Rect rect{};
		
		// Set size and position of snake part
		rect.w = global::BLOCK_SIZE;
		rect.h = global::BLOCK_SIZE;
		rect.x = static_cast<int>(p.x);
		rect.y = static_cast<int>(p.y);

		if (i + 1 == size)
		{
			// Set draw colour to red for head of snake 
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0, 0xFF);
		}
		else
		{
			// Set draw colour to white for tail
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);			
		}
		
		// Fill rectangle with set colour
		SDL_RenderFillRect(renderer, &rect);
		
		i++;
	}
}

void DrawScore(int score, SDL_Renderer* renderer, TTF_Font* font, SDL_Surface*& surface, SDL_Texture*& texture, SDL_Rect* rect)
{		
	if (surface != nullptr) {
		SDL_FreeSurface(surface);
		surface = nullptr;
	}
	if (texture != nullptr) {
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}

	surface = TTF_RenderText_Solid(font, std::to_string(score).c_str(), { 0xFF, 0xFF, 0xFF, 0xFF });
	texture = SDL_CreateTextureFromSurface(renderer, surface);

	int width, height;
	SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);	
	rect->x = static_cast<int>(global::WINDOW_WIDTH - width - 10);
	rect->y = static_cast<int>(10);
	rect->w = width;
	rect->h = height;	

	// Draw highest score
	SDL_RenderCopy(renderer, texture, nullptr, rect);
}

//void ShowGraph(int* gameCount, Agent* agent)
//{		
//	// Prepare data.
//	int n = *gameCount;
//	std::vector<double> x(n), y(n), w(n);
//	for (int i = 0; i < n; ++i) {
//		x.at(i) = i + 1;	
//		y.at(i) = scores[i];
//		w.at(i) = (float)std::accumulate(scores.begin(), scores.end() - (n - i - 1), 0) / (float)n ;
//	}
//	
//	// Create plot title
//	std::stringstream stream;
//	stream << std::fixed << std::setprecision(2) << agent->learningRate;
//	std::string alpha = stream.str();
//	
//	std::stringstream stream1;
//	stream1 << std::fixed << std::setprecision(2) << agent->discountRate;
//	std::string gamma = stream1.str();
//
//	std::stringstream stream2;
//	stream2 << std::fixed << std::setprecision(2) << std::to_string(w.back());
//	std::string avgScore = stream2.str();
//	
//	std::string title = std::to_string(n) + " games | alpha=" + alpha + " | gamma=" + gamma;
//
//	// Create filename
//	alpha.erase(remove(alpha.begin(), alpha.end(), '.'), alpha.end());
//	gamma.erase(remove(gamma.begin(), gamma.end(), '.'), gamma.end());
//
//	std::string fname = "./" + std::to_string(n) + "games" + alpha + "alpha" + gamma + "gamma" + ".png";
//
//	// Set the size of output image to 1200x780 pixels
//	plt::figure_size(1200, 780);
//	// Plot line from given x and y data. Color is selected automatically.
//	plt::named_plot("score", x, y);
//	// Plot a red dashed line from given x and y data.
//	plt::named_plot("average_score=" + avgScore, x, w, "r--");
//	// Set x-axis to interval [0, gameCount]
//	plt::xlim(0, n);
//	// Add graph title
//	plt::title(title);
//	// Enable legend.
//	plt::legend();
//	// Save the image (file format is determined by the extension)
//	plt::save(fname);
//	// Show plot
//	//plt::show();
//}

struct GameResult GameLoop(SDL_Renderer* renderer, TTF_Font* font, SDL_Surface* surface, SDL_Texture* texture, SDL_Rect* scoreRect, Agent* agent, int highScore)
{			
	// Frame time variables
	float dt = 0.004f;
	Uint64 startTicks = 0;
	Uint64 endTicks = 0;

	// Snake starting position, list of parts and length	
	Vec2 snakePos = Vec2(global::WINDOW_WIDTH / 2, global::WINDOW_HEIGHT / 2);
	Vec2 oldSnakePos = snakePos;
	Vec2 snakeHead = snakePos;
	float snakeX1Change = 0.0;
	float snakeY1Change = 0.0;
	std::vector<Vec2> snakePartsList;
	snakePartsList.push_back(snakePos);
	int snakeLength = 1;	
	int snakeDir = 0; // Begin game heading north

	// First food block position
	Vec2 foodPos = Vec2(
		round(rand() % (global::WINDOW_WIDTH - global::BLOCK_SIZE) / 10.0) * 10.0,
		round(rand() % (global::WINDOW_HEIGHT - global::BLOCK_SIZE) / 10.0) * 10.0
	);

	/* Update game state */
	
	std::string deathReason = "";
	bool dead = false;

	while (!dead)
	{
		// Number of ticks since SDL initialisation at start of frame
		startTicks = SDL_GetTicks();

		// Handle key presses
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
				dead = true;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					running = false;
					dead = true;
				}
			}
		}
		
		// Get action from learning agent
		Agent::Action action = agent->Act(&snakePartsList, &foodPos, &snakeDir);

		// Calculate change in position based on current direction and action
		switch (Direction(snakeDir))
		{
		case Direction::WEST:
			if (action == Agent::Action::LEFT)
			{
				snakeX1Change = 0.0;
				snakeY1Change = global::BLOCK_SIZE;
			}
			else if(action == Agent::Action::RIGHT)
			{
				snakeX1Change = 0.0;
				snakeY1Change = -global::BLOCK_SIZE;
			}
			else if (action == Agent::Action::STRAIGHT)
			{
				snakeX1Change = -global::BLOCK_SIZE;
				snakeY1Change = 0.0;
			}			
			break;
		case Direction::EAST:
			if (action == Agent::Action::LEFT)
			{
				snakeX1Change = 0.0;
				snakeY1Change = -global::BLOCK_SIZE;
			}
			else if (action == Agent::Action::RIGHT)
			{
				snakeX1Change = 0.0;
				snakeY1Change = global::BLOCK_SIZE;
			}
			else if (action == Agent::Action::STRAIGHT)
			{
				snakeX1Change = global::BLOCK_SIZE;
				snakeY1Change = 0.0;
			}
			break;
		case Direction::NORTH:
			if (action == Agent::Action::LEFT)
			{
				snakeX1Change = -global::BLOCK_SIZE;
				snakeY1Change = 0.0;
			}
			else if (action == Agent::Action::RIGHT)
			{
				snakeX1Change = global::BLOCK_SIZE;
				snakeY1Change = 0.0;
			}
			else if (action == Agent::Action::STRAIGHT)
			{
				snakeX1Change = 0.0;
				snakeY1Change = -global::BLOCK_SIZE;
			}
			break;
		case Direction::SOUTH:
			if (action == Agent::Action::LEFT)
			{
				snakeX1Change = global::BLOCK_SIZE;
				snakeY1Change = 0.0;
			}
			else if (action == Agent::Action::RIGHT)
			{
				snakeX1Change = -global::BLOCK_SIZE;
				snakeY1Change = 0.0;
			}
			else if (action == Agent::Action::STRAIGHT)
			{
				snakeX1Change = 0.0;
				snakeY1Change = global::BLOCK_SIZE;
			}
			break;
		default:
			break;
		}

		// Update previous position and add to parts list
		oldSnakePos = snakePos;
		snakePos += Vec2(snakeX1Change, snakeY1Change);
		snakeHead = snakePos;
		snakePartsList.push_back(snakeHead);

		// Die if collision with border of window
		if (snakeHead.x >= global::WINDOW_WIDTH || snakeHead.x < 0 || snakeHead.y >= global::WINDOW_HEIGHT || snakeHead.y < 0)
		{
			deathReason = "screen";
			dead = true;
		}
		
		// Check for collision with tail
		std::vector<Vec2> snakeTail(snakePartsList.begin(), snakePartsList.end() - 1);
		
		if(std::find(snakeTail.begin(), snakeTail.end(), snakeHead) != snakeTail.end())
		{
			deathReason = "tail";
			dead = true;
		}

		// If snake head position overlaps with food position, increase snake length
		if (snakeHead == foodPos)
		{
			foodPos = Vec2(
				round(rand() % (global::WINDOW_WIDTH - global::BLOCK_SIZE) / 10.0) * 10.0,
				round(rand() % (global::WINDOW_HEIGHT - global::BLOCK_SIZE) / 10.0) * 10.0
			);
			snakeLength++;
		}

		// If food not eaten then remove end of tail
		if (snakePartsList.size() > snakeLength)
		{
			snakePartsList.erase(snakePartsList.begin());
		}

		// Determine the direction the snake is travelling
		if (snakePos.x < oldSnakePos.x) // West
		{
			snakeDir = 0;
		}
		else if (snakePos.x > oldSnakePos.x) // East
		{
			snakeDir = 1;
		}
		else if (snakePos.y < oldSnakePos.y) // North
		{
			snakeDir = 2;
		}
		else if (snakePos.y > oldSnakePos.y) // South
		{
			snakeDir = 3;
		}		

		// Clear the window to black before rendering new frame
		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
		SDL_RenderClear(renderer);

		// Draw food and snake
		DrawFood(&foodPos, renderer);
		DrawSnake(&snakePartsList, renderer);
		DrawScore(highScore, renderer, font, surface, texture, scoreRect);

		// Present the backbuffer
		SDL_RenderPresent(renderer);

		// Update learning matrix
		agent->UpdateLM(&deathReason, &snakePartsList, &foodPos, &snakeDir);

		// Update frame time
		endTicks = SDL_GetTicks(); // Ticks since SDL initialisation at end of frame		
		dt = (endTicks - startTicks); // Calculate frame time		
		while (dt < global::FRAME_DELAY_MS) // Wait for FRAME_TIME milliseconds to pass before processing next frame
		{
			endTicks = SDL_GetTicks();
			dt = (endTicks - startTicks);
		}
	}

	GameResult result{};
	result.score = snakeLength - 1;
	result.deathReason = deathReason;

	// Add score to list for plotting
	scores.push_back(snakeLength - 1);

	return result;
}

int main(int argc, char* argv[])
{
	// Initialize SDL components
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();

	// Initialize fonts
	TTF_Font* scoreFont = TTF_OpenFont("DejaVuSansMono.ttf", 40);
	if (!scoreFont) {
		std::cout << "Failed to load font: " << TTF_GetError() << "\n";
		return 1;
	}

	// Create window and 2D rendering context
	std::string windowTitle = "SnakeAI";
	SDL_Window* window = SDL_CreateWindow(windowTitle.c_str(), 10, 40, global::WINDOW_WIDTH, global::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	
	// Create high score text
	SDL_Surface* surface = nullptr;
	SDL_Texture* texture = nullptr;
	SDL_Rect scoreRect{};
	int scoreWidth, scoreHeight;
	SDL_QueryTexture(texture, nullptr, nullptr, &scoreWidth, &scoreHeight);
	scoreRect.x = static_cast<int>(global::WINDOW_WIDTH - 40);
	scoreRect.y = static_cast<int>(10);
	scoreRect.w = scoreWidth;
	scoreRect.h = scoreHeight;
	
	// Initialise learning matrix
	InitLM();

	// Create learning agent
	Agent agent;

	// Train agent
	int gameCount = 0;
	int highScore = 0;
	GameResult result;
	while (running && gameCount < global::GAMES_LIMIT)
	{
		// Clear history of state and actions
		agent.Reset();		
		// Run game
		result = GameLoop(renderer, scoreFont, surface, texture, &scoreRect, &agent, highScore);
		// Increase number of games
		gameCount++;
		// Update high score
		if (result.score > highScore) { highScore = result.score; }
		// Decrease epsilon by fraction each game
		agent.epsilon = std::max(global::EPSILON_MIN, global::EPSILON_DECAY * agent.epsilon);
		// Results of each game
		std::cout << "Game: " << gameCount
				<< "; Score: " << result.score
				<< "; Death: " << result.deathReason
				<< "; Loops: " << agent.numLoops
				<< "; Epsilon: " << agent.epsilon
				<< "; Alpha: " << agent.learningRate
				<< "; Gamma: " << agent.discountRate
				<< std::endl;
		// Save learning matrix to file every LM_SAVE_INTERVAL 
		if (gameCount % global::LM_SAVE_INTERVAL == 0)
		{
			std::cout << "Saving learning matrix\n";
			agent.SaveLM("lm.json");
		}
	}

	// Display graph
	//ShowGraph(&gameCount, &agent);
	
	// Cleanup resources
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(scoreFont);
	TTF_Quit();
	SDL_Quit();

	return 0;
}