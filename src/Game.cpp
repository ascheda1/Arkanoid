#include "Game.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include "Utils.h"

Game::Game() {}
Game::~Game() {
    delete paddle;
    //delete ball;
    balls.clear();
    if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

bool Game::init(const char* title, int w, int h) {
    winW = w; winH = h;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        return false;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winW, winH, SDL_WINDOW_SHOWN);
    if (!window) { SDL_Log("Window create failed: %s", SDL_GetError()); return false; }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { SDL_Log("Renderer create failed: %s", SDL_GetError()); return false; }

    font = TTF_OpenFontIndex("C:/Windows/Fonts/arial.ttf", 18, 0); // default font path on Windows
    if (!font) {
        SDL_Log("Font load failed; score won't show text: %s", TTF_GetError());
        // non-fatal: still run
    }

    // create paddle and ball
    paddle = new Paddle(winW/2 - 60, winH - 40, 120, 16);
    Ball* ball = new Ball(winW/2.0f, winH - 40 - 7 - 2);
    ball->reset_on_paddle(paddle->rect);
    balls.push_back(ball);

    spawnBricks();

    running = true;
    return true;
}

void Game::spawnBricks() {
    bricks.clear();
    int rows = 6;
    int cols = 10;
    int pad = 4;
    int brickW = (winW - (cols+1)*pad) / cols;
    int brickH = 24;
    for (int r=0;r<rows;++r) {
        for (int c=0;c<cols;++c) {
            int x = pad + c*(brickW+pad);
            int y = 50 + r*(brickH + pad);
            BrickColor color;
            if (r < 2) color = BrickColor::Red;
            else if (r < 4) color = BrickColor::Green;
            else color = BrickColor::Blue;
            bricks.emplace_back(std::make_unique<Brick>(x,y,brickW,brickH,color));
        }
    }
    score = 0;
    ballsLeft = 3;
}

void Game::run() {
    Uint64 last = SDL_GetPerformanceCounter();
    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = float((now - last) / double(SDL_GetPerformanceFrequency()));
        last = now;

        handleEvents();
        update(dt);
        render();
    }
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) running = false;
        else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) running = false;
            if (e.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                if (!balls.empty() && !balls.at(0)->active) {
                    if (ballsLeft > 0) {
                        balls.at(0)->launch();
                    }
                }
            }
            if (e.key.keysym.scancode == SDL_SCANCODE_O){
                Ball* new_ball = new Ball(balls.at(0)->pos.x, balls.at(0)->pos.y);
                new_ball->active = true;
                new_ball->vel = Vec2(-balls.at(0)->vel.x, -balls.at(0)->vel.y);
                balls.push_back(new_ball);
            }
        } else if (e.type == SDL_MOUSEMOTION) {
            // optional: paddle follows mouse
            int mx = e.motion.x;
            paddle->moveToMouse(mx, winW);
            if (!balls.empty() && !balls.at(0)->active) balls.at(0)->reset_on_paddle(paddle->rect);
        }
    }
}

bool Game::allBallsNotActive(){
    for (auto ball : balls){
        if (ball->active)
        return false;
    }
    return true;
}

void Game::update(float dt) {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    paddle->update(dt, keys, winW);

    // if ball not active, keep it on paddle
    if (!balls.empty() && !balls.at(0)->active) {
        balls.at(0)->reset_on_paddle(paddle->rect);
    }

    for (auto ball : balls)
        ball->update(dt);

    // collision with walls
    for (auto ball : balls){
        if (ball->pos.x - ball->radius <= 0) {
            ball->pos.x = ball->radius;
            ball->vel.x = -ball->vel.x;
        }
        if (ball->pos.x + ball->radius >= winW) {
            ball->pos.x = winW - ball->radius;
            ball->vel.x = -ball->vel.x;
        }
        if (ball->pos.y - ball->radius <= 0) {
            ball->pos.y = ball->radius;
            ball->vel.y = -ball->vel.y;
        }
        // TODO: bottom: lost ball
        if (ball->pos.y - ball->radius > winH) {
            
            ball->active = false;
            if (allBallsNotActive())
                ballsLeft--;
            ball->reset_on_paddle(paddle->rect);
            if (ballsLeft <= 0) {
                // game over -> reset level (or stop)
                // here: reset bricks and score for simplicity
                spawnBricks();
            }
        }

        // ball-paddle collision: reflect according to hit relative position
        SDL_Rect brect = ball->bounds();
        SDL_Rect preg = paddle->rect;
        if (SDL_HasIntersection(&brect, &preg) && ball->vel.y > 0) {
            // compute hit pos relative to paddle center [-1..1]
            float hitX = (ball->pos.x - (paddle->rect.x + paddle->rect.w/2.0f)) / (paddle->rect.w/2.0f);
            hitX = clampf(hitX, -1.0f, 1.0f);
            float angle = hitX * (75.0f * float(M_PI/180.0f)); // max 75 degrees deflection
            float speed = length(ball->vel);
            ball->vel.x = sinf(angle) * speed;
            ball->vel.y = -fabs(cosf(angle) * speed); // always upward
            // move ball above paddle to prevent sticking
            ball->pos.y = paddle->rect.y - ball->radius - 1;
        }

        // ball-brick collisions
        for (auto &bptr : bricks) {
            if (!bptr->alive) continue;
            SDL_Rect br = bptr->rect;
            if (SDL_HasIntersection(&brect, &br)) {
                bptr->alive = false;
                score += bptr->points;

                // Simple collision response: determine whether collision was more horizontal or vertical
                float overlapLeft = float(brect.x + brect.w) - float(br.x);
                float overlapRight = float(br.x + br.w) - float(brect.x);
                float overlapTop = float(brect.y + brect.h) - float(br.y);
                float overlapBottom = float(br.y + br.h) - float(brect.y);
                float minOverlapX = std::min(overlapLeft, overlapRight);
                float minOverlapY = std::min(overlapTop, overlapBottom);

                if (minOverlapX < minOverlapY) {
                    ball->vel.x = -ball->vel.x;
                } else {
                    ball->vel.y = -ball->vel.y;
                }
                break; // only one brick per frame
            }
        }

        // check win condition
        bool anyAlive = false;
        for (auto &bptr : bricks) if (bptr->alive) { anyAlive = true; break; }
        if (!anyAlive) {
            // player wins -> reset level (could show message)
            spawnBricks();
        }
    }
    for (auto ball : balls){
        if (balls.at(0) == ball)
            continue;
        if (!ball->active){
            auto it = std::find(balls.begin(), balls.end(), ball);
            balls.erase(it);
        }
    }
}

void Game::render() {
    // clear
    SDL_SetRenderDrawColor(renderer, 20,20,30,255);
    SDL_RenderClear(renderer);

    // render bricks
    for (auto &b : bricks) b->render(renderer);

    // render paddle and ball
    paddle->render(renderer);
    for (auto ball : balls)
        ball->render(renderer);

    // HUD - score and balls left
    if (font) {
        SDL_Color color = { 230,230,230,255 };
        std::ostringstream ss;
        ss << "Score: " << score << "   Balls: " << ballsLeft;
        std::string s = ss.str();
        SDL_Surface* surf = TTF_RenderText_Blended(font, s.c_str(), color);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = { 8, 8, surf->w, surf->h };
            SDL_FreeSurface(surf);
            if (tex) {
                SDL_RenderCopy(renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
        }
    } else {
        // fallback: draw a small rect as score placeholder
        SDL_SetRenderDrawColor(renderer, 200,200,200,255);
        SDL_Rect r = {8,8,120,24};
        SDL_RenderFillRect(renderer, &r);
    }

    SDL_RenderPresent(renderer);
}
