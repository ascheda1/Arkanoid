#include "Game.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include "Utils.h"
#include <chrono>
#include <thread>

game::game() {}
game::~game()
{
    delete m_paddle;
    // delete ball;
    balls.clear();
    bonuses.clear();
    if (font)
        TTF_CloseFont(font);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

bool game::init(const char *title, int w, int h)
{
    winW = w;
    winH = h;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }
    if (TTF_Init() != 0)
    {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        return false;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winW, winH, SDL_WINDOW_SHOWN);
    if (!window)
    {
        SDL_Log("Window create failed: %s", SDL_GetError());
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        SDL_Log("Renderer create failed: %s", SDL_GetError());
        return false;
    }

    font = TTF_OpenFontIndex("C:/Windows/Fonts/arial.ttf", 18, 0); // default font path on Windows
    if (!font)
    {
        SDL_Log("Font load failed; score won't show text: %s", TTF_GetError());
        // non-fatal: still run
    }

    // create paddle and ball
    m_paddle = new paddle(winW / 2 - 60, winH - 40, 120, 16);
    ball *b = new ball(winW / 2.0f, winH - 40 - 7 - 2);
    b->reset_on_paddle(m_paddle->rect);
    balls.push_back(b);

    spawn_bricks();

    running = true;
    return true;
}

void game::reset_level()
{
    bonuses.clear();
    balls.clear();
    ball *b = new ball(winW / 2.0f, winH - 40 - 7 - 2);
    b->reset_on_paddle(m_paddle->rect);
    balls.push_back(b);
}

void game::spawn_bricks()
{
    bricks.clear();
    int rows = 6;
    int cols = 10;
    int pad = 4;
    int brickW = (winW - (cols + 1) * pad) / cols;
    int brickH = 24;
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            int x = pad + c * (brickW + pad);
            int y = 50 + r * (brickH + pad);
            brick_color color;
            if (r < 2)
                color = brick_color::red;
            else if (r < 4)
                color = brick_color::green;
            else
                color = brick_color::blue;
            double val = (double)rand() / RAND_MAX;
            bricks.emplace_back(std::make_unique<brick>(x, y, brickW, brickH, color, val < 0.25));
        }
    }
    score = 0;
    ballsLeft = 3;
}

void game::run()
{
    Uint64 last = SDL_GetPerformanceCounter();
    while (running)
    {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = float((now - last) / double(SDL_GetPerformanceFrequency()));
        last = now;

        handle_events();
        update(dt);
        render();
    }
}

void game::handle_events()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            running = false;
        else if (e.type == SDL_KEYDOWN)
        {
            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                running = false;
            if (e.key.keysym.scancode == SDL_SCANCODE_SPACE)
            {
                if (!balls.empty() && !balls.at(0)->active)
                {
                    if (ballsLeft > 0)
                    {
                        balls.at(0)->launch();
                    }
                }
            }
        }
        else if (e.type == SDL_MOUSEMOTION)
        {
            // optional: paddle follows mouse
            int mx = e.motion.x;
            m_paddle->move_to_mouse(mx, winW);
            if (all_balls_not_active())
            {
                balls.clear();
                ball *b = new ball(winW / 2.0f, winH - 40 - 7 - 2);
                b->reset_on_paddle(m_paddle->rect);
                balls.push_back(b);
            }
        }
    }
}

bool game::all_balls_not_active()
{
    for (auto b : balls)
    {
        if (b->active)
            return false;
    }
    return true;
}

void game::update(float dt)
{
    const Uint8 *keys = SDL_GetKeyboardState(nullptr);
    m_paddle->update(dt, keys, winW);

    for (auto bonus_ptr : bonuses)
    {
        bonus_ptr->update(dt);
    }

    // if ball not active, keep it on paddle
    if (balls.size() == 1 && !balls.at(0)->active)
    {
        balls.at(0)->reset_on_paddle(m_paddle->rect);
    }

    for (auto b : balls)
        b->update(dt);

    // collision with walls
    for (auto b : balls)
    {
        if (b->pos.x - b->radius <= 0)
        {
            b->pos.x = b->radius;
            b->vel.x = -b->vel.x;
        }
        if (b->pos.x + b->radius >= winW)
        {
            b->pos.x = winW - b->radius;
            b->vel.x = -b->vel.x;
        }
        if (b->pos.y - b->radius <= 0)
        {
            b->pos.y = b->radius;
            b->vel.y = -b->vel.y;
        }
        // bottom: lost ball
        if (b->pos.y - b->radius > winH)
        {

            b->active = false;
            if (all_balls_not_active())
            {
                ballsLeft--;
                b->reset_on_paddle(m_paddle->rect);
            }
            if (ballsLeft <= 0)
            {
                // game over -> reset level (or stop)
                // here: reset bricks and score for simplicity
                spawn_bricks();
            }
        }

        // ball-paddle collision: reflect according to hit relative position
        SDL_Rect brect = b->bounds();
        SDL_Rect preg = m_paddle->rect;
        if (SDL_HasIntersection(&brect, &preg) && b->vel.y > 0)
        {
            // compute hit pos relative to paddle center [-1..1]
            float hitX = (b->pos.x - (m_paddle->rect.x + m_paddle->rect.w / 2.0f)) / (m_paddle->rect.w / 2.0f);
            hitX = clampf(hitX, -1.0f, 1.0f);
            float angle = hitX * (75.0f * float(M_PI / 180.0f)); // max 75 degrees deflection
            float speed = length(b->vel);
            b->vel.x = sinf(angle) * speed;
            b->vel.y = -fabs(cosf(angle) * speed); // always upward
            // move ball above paddle to prevent sticking
            b->pos.y = m_paddle->rect.y - b->radius - 1;
        }

        // ball-brick collisions
        for (auto &bptr : bricks)
        {
            if (!bptr->alive)
                continue;
            SDL_Rect br = bptr->rect;
            if (SDL_HasIntersection(&brect, &br))
            {
                // bonus handle
                if (bptr->alive && bptr->m_bonus)
                {
                    bonus *new_bonus = new bonus(br.x + 24, br.y, 24, 24, bonus_type::double_ball);
                    new_bonus->alive = true;
                    bonuses.push_back(new_bonus);
                }
                bptr->alive = false;
                score += bptr->points;

                // Simple collision response: determine whether collision was more horizontal or vertical
                float overlapLeft = float(brect.x + brect.w) - float(br.x);
                float overlapRight = float(br.x + br.w) - float(brect.x);
                float overlapTop = float(brect.y + brect.h) - float(br.y);
                float overlapBottom = float(br.y + br.h) - float(brect.y);
                float minOverlapX = std::min(overlapLeft, overlapRight);
                float minOverlapY = std::min(overlapTop, overlapBottom);

                if (minOverlapX < minOverlapY)
                {
                    b->vel.x = -b->vel.x;
                }
                else
                {
                    b->vel.y = -b->vel.y;
                }
                break; // only one brick per frame
            }
        }

        // check win condition
        bool anyAlive = false;
        for (auto &bptr : bricks)
            if (bptr->alive)
            {
                anyAlive = true;
                break;
            }
        if (!anyAlive)
        {
            // player wins -> reset level (could show message)
            running = false;
            reset_level();
            // std::this_thread::sleep_for(std::chrono::milliseconds(500));
            spawn_bricks();
            running = true;
            return;
        }
    }
    for (auto b : balls)
    {
        if (balls.at(0) == b)
            continue;
        if (!b->active)
        {
            auto it = std::find(balls.begin(), balls.end(), b);
            balls.erase(it);
        }
    }

    for (auto bonus_ptr : bonuses)
    {
        SDL_Rect brect = bonus_ptr->bounds();
        SDL_Rect preg = m_paddle->rect;
        if (SDL_HasIntersection(&brect, &preg) && bonus_ptr->size.y > 0)
        {
            bonus_ptr->alive = false;
            int size = balls.size();
            for (int i = 0; i < size; ++i)
            {
                if (!balls.at(i)->active)
                    continue;
                ball *new_ball = new ball(balls.at(i)->pos.x, balls.at(i)->pos.y);
                new_ball->active = true;
                new_ball->vel = vec2(-balls.at(i)->vel.x, -balls.at(i)->vel.y);
                balls.push_back(new_ball);
            }
        }
    }

    for (auto bonus_ptr : bonuses)
    {
        if (!bonus_ptr->alive)
        {
            auto it = std::find(bonuses.begin(), bonuses.end(), bonus_ptr);
            bonuses.erase(it);
        }
    }
}

void game::render()
{
    // clear
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_RenderClear(renderer);

    // render bricks
    for (auto &b : bricks)
        b->render(renderer);

    // render paddle and ball
    m_paddle->render(renderer);
    for (auto b : balls)
        b->render(renderer);

    // render bonuses
    for (auto bonus_ptr : bonuses)
    {
        bonus_ptr->render(renderer);
    }

    // HUD - score and balls left
    if (font)
    {
        SDL_Color color = {230, 230, 230, 255};
        std::ostringstream ss;
        ss << "Score: " << score << "   Balls: " << ballsLeft;
        std::string s = ss.str();
        SDL_Surface *surf = TTF_RenderText_Blended(font, s.c_str(), color);
        if (surf)
        {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect dst = {8, 8, surf->w, surf->h};
            SDL_FreeSurface(surf);
            if (tex)
            {
                SDL_RenderCopy(renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
        }
    }
    else
    {
        // fallback: draw a small rect as score placeholder
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_Rect r = {8, 8, 120, 24};
        SDL_RenderFillRect(renderer, &r);
    }

    SDL_RenderPresent(renderer);
}