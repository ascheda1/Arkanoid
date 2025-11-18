#pragma once
#include <SDL.h>
#include "Utils.h"

class Ball {
public:
    Vec2 pos;
    Vec2 vel;
    int radius = 7;
    bool active = false; // ball is moving only when active

    Ball(float x, float y) { pos = Vec2(x,y); vel = Vec2(0,0); }

    void launch(float speed = 350.0f) {
        if (active) return;
        // initial direction: up-right
        vel = normalize(Vec2(0.4f, -1.0f)) * speed;
        active = true;
    }

    void reset_on_paddle(const SDL_Rect& paddle) {
        active = false;
        pos.x = paddle.x + paddle.w/2.0f;
        pos.y = paddle.y - radius - 1;
        vel = Vec2(0,0);
    }

    SDL_Rect bounds() const {
        return SDL_Rect{ int(pos.x - radius), int(pos.y - radius), radius*2, radius*2 };
    }

    void update(float dt) {
        if (!active) return;
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
    }

    void render(SDL_Renderer* ren) {
        // simple filled circle (approx with rect+center pixel)
        SDL_SetRenderDrawColor(ren, 240,240,60,255);
        SDL_Rect r = bounds();
        SDL_RenderFillRect(ren, &r);
        SDL_SetRenderDrawColor(ren, 0,0,0,255);
        SDL_RenderDrawRect(ren, &r);
    }
};
