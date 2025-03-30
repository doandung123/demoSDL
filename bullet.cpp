#include "bullet.h"
#include "game.h"
#include <SDL_image.h>
#include <iostream>

Bullet::Bullet(SDL_Renderer* renderer, int startX, int startY, int dirX, int dirY, double angle) {
    x = startX;
    y = startY;
    dx = dirX * BULLET_SPEED;
    dy = dirY * BULLET_SPEED;
    active = true;
    this->angle = angle;

    SDL_Surface* surface = IMG_Load("bullet.jpg");
    if (!surface) {
        std::cerr << "Không thể tải ảnh bullet.jpg! SDL_image Error: " << IMG_GetError() << std::endl;
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    rect = {x, y, 10, 10};
}

void Bullet::move() {
    x += dx;
    y += dy;
    rect.x = x;
    rect.y = y;

    if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) {
        active = false;
    }
}

void Bullet::render(SDL_Renderer* renderer) {
    if (active && texture) {
        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
    }
}
