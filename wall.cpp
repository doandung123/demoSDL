#include "wall.h"
#include <SDL_image.h>
#include <iostream>

Wall::Wall(SDL_Renderer* renderer, int startX, int startY, bool indestructible) : x(startX), y(startY), active(true), indestructible(indestructible) {
    rect = {x, y, TILE_SIZE, TILE_SIZE};
    SDL_Surface* surface = IMG_Load(indestructible ? "stone.jpg" : "wall.jpg");
    if (!surface) {
        std::cerr << "Không thể tải ảnh! SDL_image Error: " << IMG_GetError() << std::endl;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void Wall::render(SDL_Renderer* renderer) {
    if (active) {
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    }
}
