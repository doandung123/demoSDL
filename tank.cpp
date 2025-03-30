#include "tank.h"
#include <SDL_image.h>
#include <iostream>
#include <algorithm>
#include <cstdlib>

// PlayerTank
PlayerTank::PlayerTank() : x(0), y(0), dirX(0), dirY(-1), texture(nullptr) {
    rect = {x, y, TILE_SIZE, TILE_SIZE};
}

PlayerTank::PlayerTank(SDL_Renderer* renderer, int startX, int startY) : x(startX), y(startY), dirX(0), dirY(-1), shootCooldown(0) {
    rect = {x, y, TILE_SIZE, TILE_SIZE};
    SDL_Surface* surface = IMG_Load("playertank.jpg");
    if (!surface) {
        std::cerr << "Lỗi tải ảnh playertank.jpg: " << IMG_GetError() << std::endl;
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Lỗi tạo texture: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_FreeSurface(surface);
}

void PlayerTank::move(int dx, int dy, const std::vector<Wall>& walls) {
    int newX = x + dx;
    int newY = y + dy;
    SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};

    bool canMove = true;
    for (const auto& wall : walls) {
        if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) {
            canMove = false;
            break;
        }
    }

    if (canMove && newX >= TILE_SIZE && newX <= (MAP_WIDTH - 2) * TILE_SIZE &&
        newY >= TILE_SIZE && newY <= (MAP_HEIGHT - 2) * TILE_SIZE) {
        x = newX;
        y = newY;
        rect.x = x;
        rect.y = y;
    }

    if (dx == 5) {
        angle = 90;
        dirX = 1;
        dirY = 0;
    } else if (dx == -5) {
        angle = -90;
        dirX = -1;
        dirY = 0;
    } else if (dy == 5) {
        angle = 180;
        dirX = 0;
        dirY = 1;
    } else if (dy == -5) {
        angle = 0;
        dirX = 0;
        dirY = -1;
    }
}

void PlayerTank::checkAndMoveOutOfStuck(const std::vector<Wall>& walls) {
    for (const auto& wall : walls) {
        if (wall.active && SDL_HasIntersection(&rect, &wall.rect)) {
            int dx = x - wall.x;
            int dy = y - wall.y;
            move(dx, dy, walls);
            return;
        }
    }

    if (x < TILE_SIZE) {
        move(5, 0, walls);
    } else if (x > (MAP_WIDTH - 2) * TILE_SIZE) {
        move(-5, 0, walls);
    } else if (y < TILE_SIZE) {
        move(0, 5, walls);
    } else if (y > (MAP_HEIGHT - 2) * TILE_SIZE) {
        move(0, -5, walls);
    }
}

void PlayerTank::shoot(SDL_Renderer* renderer) {
    if (shootCooldown == 0) {
        bullets.push_back(Bullet(renderer, x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, angle));
        shootCooldown = 30;
    }
}

void PlayerTank::updateBullets() {
    if (shootCooldown > 0) {
        --shootCooldown;
    }
    for (auto &bullet : bullets) {
        bullet.move();
    }
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet &b) { return !b.active; }), bullets.end());
}

void PlayerTank::render(SDL_Renderer* renderer) {
    SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
    for (auto &bullet : bullets) {
        bullet.render(renderer);
    }
}

// EnemyTank
EnemyTank::EnemyTank(SDL_Renderer* renderer, int startX, int startY) {
    moveDelay = 15;
    shootDelay = 5;
    lastDirection = -1;
    moveCounter = 0;

    x = startX;
    y = startY;

    rect = {x, y, TILE_SIZE, TILE_SIZE};
    dirX = 0;
    dirY = 1;
    active = true;

    SDL_Surface* surface = IMG_Load("enemytank.jpg");
    if (!surface) {
        std::cerr << "Không thể tải ảnh enemytank.jpg! SDL_image Error: " << IMG_GetError() << std::endl;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void EnemyTank::move(const std::vector<Wall>& walls) {
    if (--moveDelay > 0) return;
    moveDelay = 15;
    int r;
    int newDirection;
    if (moveCounter > 0) {
        newDirection = lastDirection;
        moveCounter--;
    } else {
        do {
            r = rand() % 4;
            newDirection = r;
        } while (newDirection == (lastDirection + 2) % 4);

        lastDirection = newDirection;
        moveCounter = rand() % 5 + 3;
    }

    if (newDirection == 0) {
        dirX = 0;
        dirY = -5;
        angle = 0;
    } else if (newDirection == 1) {
        dirX = 0;
        dirY = 5;
        angle = 180;
    } else if (newDirection == 2) {
        dirY = 0;
        dirX = -5;
        angle = -90;
    } else if (newDirection == 3) {
        dirY = 0;
        dirX = 5;
        angle = 90;
    }

    int newX = x + dirX;
    int newY = y + dirY;
    SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};

    if (newX < TILE_SIZE || newX > (MAP_WIDTH - 2) * TILE_SIZE ||
        newY < TILE_SIZE || newY > (MAP_HEIGHT - 2) * TILE_SIZE) {
        if (newX < TILE_SIZE) {
            newDirection = 3;
        } else if (newX > (MAP_WIDTH - 2) * TILE_SIZE) {
            newDirection = 2;
        } else if (newY < TILE_SIZE) {
            newDirection = 1;
        } else if (newY > (MAP_HEIGHT - 2) * TILE_SIZE) {
            newDirection = 0;
        }

        lastDirection = newDirection;
        moveCounter = rand() % 5 + 3;
        return;
    }

    for (const auto& wall : walls) {
        if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) {
            return;
        }
    }

    if (newX >= TILE_SIZE && newX <= (MAP_WIDTH - 2) * TILE_SIZE &&
        newY >= TILE_SIZE && newY <= (MAP_HEIGHT - 2) * TILE_SIZE) {
        x = newX;
        y = newY;
        rect.x = x;
        rect.y = y;
    }
}

void EnemyTank::shoot(SDL_Renderer* renderer) {
    if (--shootDelay > 0) return;
    shootDelay = 5;

    bullets.push_back(Bullet(renderer, x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX / 5, dirY / 5, angle));
}

void EnemyTank::updateBullets() {
    for (auto& bullet : bullets) {
        bullet.move();
    }
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](Bullet& b) { return !b.active; }), bullets.end());
}

void EnemyTank::render(SDL_Renderer* renderer) {
    SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
    for (auto& bullet : bullets) {
        bullet.render(renderer);
    }
}
