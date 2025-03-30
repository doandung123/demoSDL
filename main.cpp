#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;
const int BULLET_SPEED = 3; // Giảm tốc độ đạn

class Bullet {
public:
    int x, y;
    int dx, dy;
    SDL_Rect rect;
    SDL_Texture* texture;
    bool active;
    double angle;

    Bullet(SDL_Renderer* renderer, int startX, int startY, int dirX, int dirY, double angle) {
        x = startX;
        y = startY;
        dx = dirX * BULLET_SPEED;
        dy = dirY * BULLET_SPEED;
        active = true;
        this->angle = angle;

        SDL_Surface* surface = IMG_Load("bullet.jpg");
        if (!surface) {
            cerr << "Không thể tải ảnh bullet.jpg! SDL_image Error: " << IMG_GetError() << endl;
            return;
        }
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        rect = {x, y, 10, 10};
    }

    void move() {
        x += dx;
        y += dy;
        rect.x = x;
        rect.y = y;

        if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) {
            active = false;
        }
    }

    void render(SDL_Renderer* renderer) {
        if (active && texture) {
            SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
        }
    }
};

class Wall {
public:
    int x, y;
    SDL_Rect rect;
    SDL_Texture* texture;
    bool active;
    bool indestructible; // Thêm biến để kiểm tra tường có phá hủy được không

    Wall(SDL_Renderer* renderer, int startX, int startY, bool indestructible = false) : x(startX), y(startY), active(true), indestructible(indestructible) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        SDL_Surface* surface = IMG_Load(indestructible ? "stone.jpg" : "wall.jpg");
        if (!surface) {
            cerr << "Không thể tải ảnh! SDL_image Error: " << IMG_GetError() << endl;
        }
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    void render(SDL_Renderer* renderer) {
        if (active) {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }
    }
};

class PlayerTank {
public:
    int x, y;
    SDL_Rect rect;
    SDL_Texture* texture;
    vector<Bullet> bullets;
    int dirX, dirY;
    int shootCooldown;
    double angle;

    PlayerTank() : x(0), y(0), dirX(0), dirY(-1), texture(nullptr) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
    }

    PlayerTank(SDL_Renderer* renderer, int startX, int startY) : x(startX), y(startY), dirX(0), dirY(-1), shootCooldown(0) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        SDL_Surface* surface = IMG_Load("playertank.jpg");
        if (!surface) {
            cerr << "Lỗi tải ảnh playertank.jpg: " << IMG_GetError() << endl;
            return;
        }
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            cerr << "Lỗi tạo texture: " << SDL_GetError() << endl;
            return;
        }
        SDL_FreeSurface(surface);
    }

    void move(int dx, int dy, const vector<Wall>& walls) {
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

    // hàm kiểm tra và di chuyển ra khỏi vị trí kẹt
    void checkAndMoveOutOfStuck(const vector<Wall>& walls) {
        // Kiểm tra va chạm với tường
        for (const auto& wall : walls) {
            if (wall.active && SDL_HasIntersection(&rect, &wall.rect)) {
                // Di chuyển ra khỏi tường theo hướng ngược lại
                int dx = x - wall.x;
                int dy = y - wall.y;
                move(dx, dy, walls);
                return;
            }
        }

        // Kiểm tra ra ngoài map
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

    void shoot(SDL_Renderer* renderer) {
        if (shootCooldown == 0) {
            bullets.push_back(Bullet(renderer, x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX, dirY, angle));
            shootCooldown = 30;
        }
    }
    void updateBullets() {
        if (shootCooldown > 0) {
            --shootCooldown;
        }
        for (auto &bullet : bullets) {
            bullet.move();
        }
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet &b) { return !b.active; }), bullets.end());
    }

    void render(SDL_Renderer* renderer) {
        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
        for (auto &bullet : bullets) {
            bullet.render(renderer);
        }
    }
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
    vector<Bullet> bullets;
    int lastDirection; // 0: up, 1: down, 2: left, 3: right
    int moveCounter;

    EnemyTank(SDL_Renderer* renderer, int startX, int startY) {
        moveDelay = 15;
        shootDelay = 5;
        lastDirection = -1; // Khởi tạo hướng cuối cùng
        moveCounter = 0;

        x = startX;
        y = startY;

        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirX = 0;
        dirY = 1;
        active = true;

        SDL_Surface* surface = IMG_Load("enemytank.jpg");
        if (!surface) {
            cerr << "Không thể tải ảnh enemytank.jpg! SDL_image Error: " << IMG_GetError() << endl;
        }
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    void move(const vector<Wall>& walls) {
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
            } while (newDirection == (lastDirection + 2) % 4); // Tránh di chuyển ngược lại

            lastDirection = newDirection;
            moveCounter = rand() % 5 + 3; // Số lần di chuyển theo cùng một hướng
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

       // Kiểm tra biên giới map
        if (newX < TILE_SIZE || newX > (MAP_WIDTH - 2) * TILE_SIZE ||
            newY < TILE_SIZE || newY > (MAP_HEIGHT - 2) * TILE_SIZE) {
            // Xác định hướng di chuyển hợp lệ
            if (newX < TILE_SIZE) {
                newDirection = 3; // Di chuyển sang phải
            } else if (newX > (MAP_WIDTH - 2) * TILE_SIZE) {
                newDirection = 2; // Di chuyển sang trái
            } else if (newY < TILE_SIZE) {
                newDirection = 1; // Di chuyển xuống dưới
            } else if (newY > (MAP_HEIGHT - 2) * TILE_SIZE) {
                newDirection = 0; // Di chuyển lên trên
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

    void shoot(SDL_Renderer* renderer) {
        if (--shootDelay > 0) return;
        shootDelay = 5;

        bullets.push_back(Bullet(renderer, x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, dirX/5, dirY/5, angle));
    }

    void updateBullets() {
        for (auto& bullet : bullets) {
            bullet.move();
        }
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](Bullet& b) { return !b.active; }), bullets.end());
    }

    void render(SDL_Renderer* renderer) {
        SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
        for (auto& bullet : bullets) {
            bullet.render(renderer);
        }
    }
};

// Hàm vẽ ký tự bằng các hình vuông nhỏ
void drawChar(SDL_Renderer* renderer, char c, int x, int y, int r, int g, int b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    switch (c) {
        case 'A':
            // Vẽ chữ "A" bằng các hình vuông nhỏ
            SDL_Rect rect1 = {x + 2, y, 6, 2};
            SDL_Rect rect2 = {x, y + 2, 2, 6};
            SDL_Rect rect3 = {x + 8, y + 2, 2, 6};
            SDL_Rect rect4 = {x + 2, y + 8, 6, 2};
            SDL_RenderFillRect(renderer, &rect1);
            SDL_RenderFillRect(renderer, &rect2);
            SDL_RenderFillRect(renderer, &rect3);
            SDL_RenderFillRect(renderer, &rect4);
            break;
        // ... Các ký tự khác
    }
}

// Hàm vẽ văn bản bằng các hình vuông nhỏ
void drawText(SDL_Renderer* renderer, const char* text, int x, int y, int r, int g, int b) {
    for (int i = 0; text[i] != '\0'; ++i) {
        drawChar(renderer, text[i], x + i * 10, y, r, g, b);
    }
}

class Game {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    vector<Wall> walls;
    PlayerTank player;
    int enemyNumber = 3;
    vector<EnemyTank> enemies;
    int gameState; // 0: menu, 1: đang chơi, 2: thắng, 3: thua
    int endTimer; // Bộ đếm thời gian kết thúc
    SDL_Texture* winTexture;
    SDL_Texture* loseTexture;
    SDL_Rect winRect;
    SDL_Rect loseRect;
    SDL_Texture* menuTexture;
    SDL_Rect menuRect;

    Game() {
        running = true;
        gameState = 0;

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }

        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
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

    void loadTextures() {
        SDL_Surface* winSurface = IMG_Load("youwin.png");
        if (!winSurface) {
            cerr << "Lỗi tải ảnh youwin.png: " << IMG_GetError() << endl;
            return;
        }
        winTexture = SDL_CreateTextureFromSurface(renderer, winSurface);
        SDL_FreeSurface(winSurface);

        SDL_Surface* loseSurface = IMG_Load("youlose.png");
        if (!loseSurface) {
            cerr << "Lỗi tải ảnh youlose.png: " << IMG_GetError() << endl;
            return;
        }
        loseTexture = SDL_CreateTextureFromSurface(renderer, loseSurface);
        SDL_FreeSurface(loseSurface);

        winRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        loseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    }

    void loadMenuTexture() {
        SDL_Surface* surface = IMG_Load("menu.png");
        if (!surface) {
            cerr << "Lỗi tải ảnh menu.png: " << IMG_GetError() << endl;
            return;
        }
        menuTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    void generateWalls() {
        walls.clear();
        const int mapWidth = MAP_WIDTH;
        const int mapHeight = MAP_HEIGHT;

        // Tỷ lệ tạo tường (điều chỉnh để tăng hoặc giảm mật độ tường)
        const float wallDensity = 0.2f; // 20% ô có tường
        const float stoneDensity = 0.05f; // Tỷ lệ đá

        for (int i = 0; i < mapHeight; i++) {
            for (int j = 0; j < mapWidth; j++) {
                // Loại trừ biên của bản đồ
                if (i == 0 || i == mapHeight - 1 || j == 0 || j == mapWidth - 1) {
                    walls.emplace_back(renderer, j * TILE_SIZE, i * TILE_SIZE);
                } else {
                    if ((float)rand() / RAND_MAX < stoneDensity) {
                        walls.emplace_back(renderer, j * TILE_SIZE, i * TILE_SIZE, true); // Tạo đá
                    } else if ((float)rand() / RAND_MAX < wallDensity) {
                        walls.emplace_back(renderer, j * TILE_SIZE, i * TILE_SIZE); // Tạo tường thường
                    }
                }
            }
        }
    }

    void spawnEnemies() {
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

    void update() {
        player.updateBullets();
        for (auto& enemy : enemies) {
            enemy.move(walls);
            enemy.updateBullets();
            if (rand() % 100 < 2) {
                enemy.shoot(renderer);
            }
        }

        // Kiểm tra va chạm giữa PlayerTank và EnemyTank
        for (auto& enemy : enemies) {
            if (enemy.active && SDL_HasIntersection(&player.rect, &enemy.rect)) {
                // Nếu có va chạm, di chuyển PlayerTank ra khỏi EnemyTank
                int dx = player.x - enemy.x;
                int dy = player.y - enemy.y;
                player.x += dx / 2;
                player.y += dy / 2;
                player.rect.x = player.x;
                player.rect.y = player.y;
                // Kiểm tra và di chuyển ra khỏi vị trí kẹt sau khi bị đẩy
                player.checkAndMoveOutOfStuck(walls);
            }
        }

        // Kiểm tra va chạm giữa EnemyTank và EnemyTank
        for (size_t i = 0; i < enemies.size(); ++i) {
            for (size_t j = i + 1; j < enemies.size(); ++j) {
                if (enemies[i].active && enemies[j].active && SDL_HasIntersection(&enemies[i].rect, &enemies[j].rect)) {
                    // Nếu có va chạm, di chuyển EnemyTank ra khỏi EnemyTank một cách nhẹ nhàng
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
                    if (!wall.indestructible) { // Kiểm tra xem tường có phá hủy được không
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
            gameState = 2; // Thắng
            endTimer = 120; // Khởi tạo bộ đếm thời gian
        }

        for (auto& enemy : enemies) {
            for (auto& bullet : enemy.bullets) {
                if (SDL_HasIntersection(&bullet.rect, &player.rect)) {
                    gameState = 3; // Thua
                    endTimer = 120; // Khởi tạo bộ đếm thời gian
                    return;
                }
            }
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // boundaries
        SDL_RenderClear(renderer); // delete color

        if (gameState == 0) {
            drawMenu(); // Vẽ menu
        } else if (gameState == 1) {
            // Vẽ trò chơi
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
                gameState = 0; // Quay lại menu
                generateWalls(); // Tạo lại tường
                spawnEnemies(); // Tạo lại xe tăng địch
            }
        }
        SDL_RenderPresent(renderer);
    }

    void handleEvents() {
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
                    // Đang ở menu chính
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
                        // Nhấp vào nút "Start"
                        gameState = 1; // Bắt đầu chơi
                        generateWalls(); // Tạo lại tường
                        spawnEnemies(); // Tạo lại xe tăng địch
                    } else if (mouseX >= quitRect.x && mouseX <= quitRect.x + quitRect.w &&
                               mouseY >= quitRect.y && mouseY <= quitRect.y + quitRect.h) {
                        // Nhấp vào nút "Quit"
                        running = false;
                    }
                }
            } else if (event.type == SDL_KEYDOWN) {
                // Người chơi nhấn phím
                if (gameState == 1) {
                    // Đang chơi
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

    void drawMenu() {
        SDL_RenderCopy(renderer, menuTexture, NULL, &menuRect);
    }

    void run() {
        while (running) {
            handleEvents();
            update();
            render();
            SDL_Delay(16);  // Tốc độ khung hình
        }
    }

    ~Game() {
        SDL_DestroyTexture(menuTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    IMG_Init(IMG_INIT_PNG);
    Game game;
    if (game.running) {
        game.run();
    }
    IMG_Quit();
    return 0;
}
