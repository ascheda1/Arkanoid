# Arkanoid (C++ & SDL2)

A simple Arkanoid clone written in C++ using SDL2.  
Paddle, ball, bricks, scoring, lives, and basic collision mechanics are implemented.

## Features
- Player-controlled paddle (keyboard or mouse)
- Ball physics with reflections
- Multiple rows of bricks with different colors and scores
- Score and remaining lives displayed on screen
- Game restarts when player wins or runs out of balls

## Build Instructions (Windows)

### Requirements
- CMake
- vcpkg
- Visual Studio Build Tools (MSVC)

### Install dependencies
```bash
vcpkg install sdl2 sdl2-ttf
