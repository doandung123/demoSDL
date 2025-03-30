#include "game.h"
#include "utility.h"
#include <iostream>
#include <ctime>
#include <algorithm>

Game::Game() {
    running = true;
    gameState = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
        return;
    }

    window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        running = false;
        return;
    }

    player = PlayerTank(renderer, ((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE);

    srand(time(0));
    generateWalls();
    spawnEnemies();
    loadMenuTexture();
    loadTextures();
    menuRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
}

void Game::loadTextures() {
    SDL_Surface* winSurface = IMG_Load("youwin.png");
    if (!winSurface) {
        std::cerr << "Lỗi tải ảnh youwin.png: " << IMG_GetError() << std::endl;
        return;
    }
    winTexture = SDL_CreateTextureFromSurface(renderer, winSurface);
    SDL_FreeSurface(winSurface);

    SDL_Surface* loseSurface = IMG_Load("youlose.png");
    if (!loseSurface) {
        std::cerr << "Lỗi tải ảnh youlose.png: " << IMG_GetError() << std::endl;
        return;
    }
    loseTexture = SDL_CreateTextureFromSurface(renderer, loseSurface);
    SDL_FreeSurface(loseSurface);

    winRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    loseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
}

void Game::loadMenuTexture() {
    SDL_Surface* surface = IMG_Load("menu.png");
    if (!surface) {
        std::cerr << "Lỗi tải ảnh menu.png: " << IMG_GetError() << std::endl;
        return;
    }
    menuTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void Game::generateWalls() {
    walls.clear();
    const int mapWidth = MAP_WIDTH;
    const int mapHeight = MAP_HEIGHT;

    const float wallDensity = 0.2f;
    const float stoneDensity = 0.05f;

    for (int i = 0; i < mapHeight; i++) {
        for (int j = 0; j < mapWidth; j++) {
            if (i == 0 || i == mapHeight - 1 || j == 0 || j == mapWidth - 1) {
                walls.emplace_back(renderer, j * TILE_SIZE, i * TILE_SIZE, false);
            } else {
                if ((float)rand() / RAND_MAX < stoneDensity) {
                    walls.emplace_back(renderer, j * TILE_SIZE, i * TILE_SIZE, true);
                } else if ((float)rand() / RAND_MAX < wallDensity) {
                    walls.emplace_back(renderer, j * TILE_SIZE, i * TILE_SIZE, false);
                }
            }
        }
    }
}

void Game::spawnEnemies() {
    enemies.clear();
    for (int i = 0; i < enemyNumber; ++i) {
        int ex, ey;
        bool validPosition = false;
        while (!validPosition) {
            ex = (rand() % (MAP_WIDTH - 2) + 1) * TILE_SIZE;
            ey = (rand() % (MAP_HEIGHT - 2) + 1) * TILE_SIZE;
            validPosition = true;
            for (const auto& wall : walls) {
                if (wall.active && wall.x == ex && wall.y == ey) {
                    validPosition = false;
                    break;
                }
            }
        }
        enemies.push_back(EnemyTank(renderer, ex, ey));
    }
}

void Game::update() {
    player.updateBullets();
    for (auto& enemy : enemies) {
        enemy.move(walls);
        enemy.updateBullets();
        if (rand() % 100 < 2) {
            enemy.shoot(renderer);
        }
    }

    for (auto& enemy : enemies) {
        if (enemy.active && SDL_HasIntersection(&player.rect, &enemy.rect)) {
            int dx = player.x - enemy.x;
            int dy = player.y - enemy.y;
            player.x += dx / 2;
            player.y += dy / 2;
            player.rect.x = player.x;
            player.rect.y = player.y;
            player.checkAndMoveOutOfStuck(walls);
        }
    }

    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            if (enemies[i].active && enemies[j].active && SDL_HasIntersection(&enemies[i].rect, &enemies[j].rect)) {
                int dx = enemies[i].x - enemies[j].x;
                int dy = enemies[i].y - enemies[j].y;
                enemies[i].x += dx / 4;
                enemies[i].y += dy / 4;
                enemies[j].x -= dx / 4;
                enemies[j].y -= dy / 4;
                enemies[i].rect.x = enemies[i].x;
                enemies[i].rect.y = enemies[i].y;
                enemies[j].rect.x = enemies[j].x;
                enemies[j].rect.y = enemies[j].y;
            }
        }
    }

    for (auto& bullet : player.bullets) {
        for (auto& wall : walls) {
            if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                if (!wall.indestructible) {
                    wall.active = false;
                }
                bullet.active = false;
                break;
            }
        }
    }

    for (auto& bullet : player.bullets) {
        for (auto& enemy : enemies) {
            if (enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                enemy.active = false;
                bullet.active = false;
            }
        }
    }

    for (auto& enemy : enemies) {
        for (auto& bullet : enemy.bullets) {
            for (auto& wall : walls) {
                if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                    if (!wall.indestructible){
                        wall.active = false;
                    }
                    bullet.active = false;
                    break;
                }
            }
        }
    }

    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](EnemyTank& e) { return !e.active; }), enemies.end());

    if (enemies.empty()) {
        gameState = 2;
        endTimer = 120;
    }

    for (auto& enemy : enemies) {
        for (auto& bullet : enemy.bullets) {
            if (SDL_HasIntersection(&bullet.rect, &player.rect)) {
                gameState = 3;
                endTimer = 120;
                return;
            }
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    if (gameState == 0) {
        drawMenu();
    } else if (gameState == 1) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int i = 1; i < MAP_HEIGHT - 1; ++i) {
            for (int j = 1; j < MAP_WIDTH - 1; ++j) {
                SDL_Rect tile = { j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderFillRect(renderer, &tile);
            }
        }
        for (int i = 0; i < int(walls.size()); ++i) {
            walls[i].render(renderer);
        }
        player.render(renderer);
        for (auto& enemy : enemies) {
            enemy.render(renderer);
        }
    } else if (gameState == 2 && endTimer > 0) {
        SDL_RenderCopy(renderer, winTexture, NULL, &winRect);
    } else if (gameState == 3 && endTimer > 0) {
        SDL_RenderCopy(renderer, loseTexture, NULL, &loseRect);
    }

    if (gameState == 2 || gameState == 3) {
        if (endTimer > 0) {
            endTimer--;
        } else {
            gameState = 0;
            generateWalls();
            spawnEnemies();
        }
    }
    SDL_RenderPresent(renderer);
}

void Game::handleEvents() {
    SDL_Event event;
    int buttonWidth = SCREEN_WIDTH / 3;
    int buttonHeight = SCREEN_HEIGHT / 10;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            if (gameState == 0) {
                SDL_Rect startRect = {
                    SCREEN_WIDTH / 2 - buttonWidth / 2,
                    SCREEN_HEIGHT * 3 / 5,
                    buttonWidth,
                    buttonHeight
                };
                SDL_Rect quitRect = {
                    SCREEN_WIDTH / 2 - buttonWidth / 2,
                    SCREEN_HEIGHT * 3 / 5 + buttonHeight + 20,
                    buttonWidth,
                    buttonHeight
                };

                if (mouseX >= startRect.x && mouseX <= startRect.x + startRect.w &&
                    mouseY >= startRect.y && mouseY <= startRect.y + startRect.h) {
                    gameState = 1;
                    generateWalls();
                    spawnEnemies();
                } else if (mouseX >= quitRect.x && mouseX <= quitRect.x + quitRect.w &&
                    mouseY >= quitRect.y && mouseY <= quitRect.y + quitRect.h) {
                    running = false;
                }
            }
        } else if (event.type == SDL_KEYDOWN) {
            if (gameState == 1) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        player.move(0, -5, walls);
                        break;
                    case SDLK_DOWN:
                        player.move(0, 5, walls);
                        break;
                    case SDLK_LEFT:
                        player.move(-5, 0, walls);
                        break;
                    case SDLK_RIGHT:
                        player.move(5, 0, walls);
                        break;
                    case SDLK_SPACE:
                        player.shoot(renderer);
                        break;
                }
            }
        }
    }
}

void Game::drawMenu() {
    SDL_RenderCopy(renderer, menuTexture, NULL, &menuRect);
}

void Game::run() {
    while (running) {
        handleEvents();
        update();
        render();
        SDL_Delay(16);
    }
}

Game::~Game() {
    SDL_DestroyTexture(menuTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
