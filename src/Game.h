#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <memory>
#include "Paddle.h"
#include "Ball.h"
#include "Brick.h"
#include "Bonus.h"

class game
{
public:
    game();
    ~game();

    bool init(const char *title, int w, int h);
    void run();

private:
    void handle_events();
    void update(float dt);
    void render();
    void reset_level();
    void spawn_bricks();
    bool all_balls_not_active();

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    TTF_Font *font = nullptr;

    int winW = 800;
    int winH = 600;

    bool running = false;
    paddle *m_paddle = nullptr;
    // ball* ball = nullptr;
    std::vector<ball *> balls;
    std::vector<std::unique_ptr<brick>> bricks;
    std::vector<bonus *> bonuses;

    int score = 0;
    int ballsLeft = 3;
};