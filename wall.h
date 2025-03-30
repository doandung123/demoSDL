#ifndef WALL_H
#define WALL_H

#include <SDL.h>

const int TILE_SIZE = 40;

class Wall {
public:
    int x, y;
    SDL_Rect rect;
    SDL_Texture* texture;
    bool active;
    bool indestructible;

    Wall(SDL_Renderer* renderer, int startX, int startY, bool indestructible);
    void render(SDL_Renderer* renderer);
};

#endif // WALL_H
