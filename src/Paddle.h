#pragma once
#include <SDL.h>
#include "Utils.h"

class paddle
{
public:
    SDL_Rect rect;
    float speed = 600.0f; // px/s

    paddle(int x, int y, int w, int h)
    {
        rect = {x, y, w, h};
    }

    void update(float dt, const Uint8 *keys, int winW)
    {
        float move = 0.0f;
        if (keys[SDL_SCANCODE_LEFT])
            move -= 1.0f;
        if (keys[SDL_SCANCODE_RIGHT])
            move += 1.0f;

        rect.x = int(clampf(rect.x + move * speed * dt, 0.0f, float(winW - rect.w)));
    }

    void move_to_mouse(int mx, int winW)
    {
        // center paddle on mouse x
        int nx = mx - rect.w / 2;
        rect.x = int(clampf(nx, 0.0f, float(winW - rect.w)));
    }

    void render(SDL_Renderer *ren)
    {
        SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
        SDL_RenderFillRect(ren, &rect);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderDrawRect(ren, &rect);
    }
};