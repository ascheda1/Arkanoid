#pragma once
#include <SDL.h>

enum class BrickColor { Red, Green, Blue };

class Brick {
public:
    SDL_Rect rect;
    bool alive = true;
    BrickColor color;
    int points;

    Brick(int x, int y, int w, int h, BrickColor c) {
        rect = {x,y,w,h};
        color = c;
        switch(c) {
            case BrickColor::Red: points = 100; break;
            case BrickColor::Green: points = 50; break;
            case BrickColor::Blue: points = 20; break;
        }
    }

    void render(SDL_Renderer* ren) {
        if (!alive) return;
        switch(color) {
            case BrickColor::Red: SDL_SetRenderDrawColor(ren, 200,30,30,255); break;
            case BrickColor::Green: SDL_SetRenderDrawColor(ren, 30,180,30,255); break;
            case BrickColor::Blue: SDL_SetRenderDrawColor(ren, 30,120,200,255); break;
        }
        SDL_RenderFillRect(ren, &rect);
        // border
        SDL_SetRenderDrawColor(ren, 0,0,0,255);
        SDL_RenderDrawRect(ren, &rect);
    }
};
