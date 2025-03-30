#ifndef TANK_H
#define TANK_H

#include <SDL.h>
#include <vector>
#include "bullet.h"
#include "wall.h"

const int MAP_WIDTH = 800 / 40;
const int MAP_HEIGHT = 600 / 40;

class PlayerTank {
public:
    int x, y;
    SDL_Rect rect;
    SDL_Texture* texture;
    std::vector<Bullet> bullets;
    int dirX, dirY;
    int shootCooldown;
    double angle;

    PlayerTank();
    PlayerTank(SDL_Renderer* renderer, int startX, int startY);
    void move(int dx, int dy, const std::vector<Wall>& walls);
    void checkAndMoveOutOfStuck(const std::vector<Wall>& walls);
    void shoot(SDL_Renderer* renderer);
    void updateBullets();
    void render(SDL_Renderer* renderer);
};

class EnemyTank {
public:
    int x, y;
    int dirX, dirY;
    double angle;
    int moveDelay, shootDelay;
    SDL_Rect rect;
    SDL_Texture* texture;
    bool active;
    std::vector<Bullet> bullets;
    int lastDirection;
    int moveCounter;

    EnemyTank(SDL_Renderer* renderer, int startX, int startY);
    void move(const std::vector<Wall>& walls);
    void shoot(SDL_Renderer* renderer);
    void updateBullets();
    void render(SDL_Renderer* renderer);
};

#endif // TANK_H
