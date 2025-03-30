#ifndef UTILITY_H
#define UTILITY_H

#include <SDL.h>

void drawChar(SDL_Renderer* renderer, char c, int x, int y, int r, int g, int b);
void drawText(SDL_Renderer* renderer, const char* text, int x, int y, int r, int g, int b);

#endif // UTILITY_H
