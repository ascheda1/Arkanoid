#pragma once
#include <SDL.h>
#include "Utils.h"

enum class bonus_type
{
    double_ball,
    paddle_speed_up
};

class bonus
{
public:
    vec2 pos;
    vec2 size;
    bonus_type type;
    bool alive;
    float fallSpeed = 100.0f;

    bonus(int x, int y, int w, int h, bonus_type t)
    {
        pos = vec2(x, y);
        size = vec2(w, h);
        type = t;
    }

    void render(SDL_Renderer *ren)
    {
        if (!alive)
            return;

        SDL_SetRenderDrawColor(ren, 200, 30, 200, 255);
        SDL_Rect r = bounds();
        SDL_RenderFillRect(ren, &r);
    }

    void update(float dt)
    {
        if (!alive)
            return;
        pos.y += fallSpeed * dt;
    }

    SDL_Rect bounds() const
    {
        return SDL_Rect{int(pos.x), int(pos.y), int(size.x), int(size.y)};
    }
};