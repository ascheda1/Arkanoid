#pragma once
#include <SDL.h>

enum class brick_color
{
    red,
    green,
    blue
};

class brick
{
public:
    SDL_Rect rect;
    bool alive = true;
    brick_color color;
    int points;
    bool m_bonus;

    brick(int x, int y, int w, int h, brick_color c, bool bonus)
    {
        rect = {x, y, w, h};
        color = c;
        m_bonus = bonus;
        switch (c)
        {
        case brick_color::red:
            points = 100;
            break;
        case brick_color::green:
            points = 50;
            break;
        case brick_color::blue:
            points = 20;
            break;
        }
    }

    void render(SDL_Renderer *ren)
    {
        if (!alive)
            return;
        switch (color)
        {
        case brick_color::red:
            SDL_SetRenderDrawColor(ren, 200, 30, 30, 255);
            break;
        case brick_color::green:
            SDL_SetRenderDrawColor(ren, 30, 180, 30, 255);
            break;
        case brick_color::blue:
            SDL_SetRenderDrawColor(ren, 30, 120, 200, 255);
            break;
        }
        SDL_RenderFillRect(ren, &rect);
        // border
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderDrawRect(ren, &rect);
    }
};