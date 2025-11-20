#include "Game.h"

int main(int argc, char **argv)
{
    game g;
    if (!g.init("Arkanoid", 800, 600))
    {
        return -1;
    }
    g.run();
    return 0;
}