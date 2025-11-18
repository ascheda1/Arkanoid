#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <memory>
#include "Paddle.h"
#include "Ball.h"
#include "Brick.h"

class Game {
public:
    Game();
    ~Game();

    bool init(const char* title, int w, int h);
    void run();

private:
    void handleEvents();
    void update(float dt);
    void render();
    void resetLevel();
    void spawnBricks();
    bool allBallsNotActive();

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    int winW = 800;
    int winH = 600;

    bool running = false;
    Paddle* paddle = nullptr;
    //Ball* ball = nullptr;
    std::vector<Ball*> balls;
    std::vector<std::unique_ptr<Brick>> bricks;

    int score = 0;
    int ballsLeft = 3;
};
