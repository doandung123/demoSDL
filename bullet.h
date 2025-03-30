#ifndef BULLET_H
#define BULLET_H

#include <SDL.h>

const int BULLET_SPEED = 3;

class Bullet {
public:
    int x, y;
    int dx, dy;
    SDL_Rect rect;
    SDL_Texture* texture;
    bool active;
    double angle;

    Bullet(SDL_Renderer* renderer, int startX, int startY, int dirX, int dirY, double angle);
    void move();
    void render(SDL_Renderer* renderer);
};

#endif // BULLET_H
