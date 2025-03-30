#include "utility.h"

void drawChar(SDL_Renderer* renderer, char c, int x, int y, int r, int g, int b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    switch (c) {
        case 'A':
            SDL_Rect rect1 = {x + 2, y, 6, 2};
            SDL_Rect rect2 = {x, y + 2, 2, 6};
            SDL_Rect rect3 = {x + 8, y + 2, 2, 6};
            SDL_Rect rect4 = {x + 2, y + 8, 6, 2};
            SDL_RenderFillRect(renderer, &rect1);
            SDL_RenderFillRect(renderer, &rect2);
            SDL_RenderFillRect(renderer, &rect3);
            SDL_RenderFillRect(renderer, &rect4);
            break;
        // Thêm các ký tự khác vào đây
    }
}

void drawText(SDL_Renderer* renderer, const char* text, int x, int y, int r, int g, int b) {
    for (int i = 0; text[i] != '\0'; ++i) {
        drawChar(renderer, text[i], x + i * 10, y, r, g, b);
    }
}
