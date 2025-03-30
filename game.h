#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include "tank.h"
#include "wall.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

class Game {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    std::vector<Wall> walls;
    PlayerTank player;
    int enemyNumber;
    std::vector<EnemyTank> enemies;
    int gameState;
    int endTimer;
    SDL_Texture* winTexture;
    SDL_Texture* loseTexture;
    SDL_Rect winRect;
    SDL_Rect loseRect;
    SDL_Texture* menuTexture;
    SDL_Rect menuRect;

    Game();
    void loadTextures();
    void loadMenuTexture();
    void generateWalls();
    void spawnEnemies();
    void update();
    void render();
    void handleEvents();
    void drawMenu();
    void run();
    ~Game();
};

#endif // GAME_H
