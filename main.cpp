#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <ctime>
using namespace std;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT =800;
const int TILE_SIZE = 40;
const int PLAYER_RADIUS = 16;
const int BULLET_SIZE = 8;
const int SPEED = 5;
const int BULLET_SPEED = 10;
const int DUNGEON_SIZE = 20;
const int ENEMY_SPAWN_INTERVAL = 2000; // Spawn kẻ địch mỗi 2 giây

struct Room {
    int x, y;
};
    Uint32 spawnRoomChangeTime = 0; // Lưu thời điểm bắt đầu game
    bool isSpawnRoomRed = false;    // Trạng thái đổi màu

vector<Room> dungeons = {{0, 0}, {50, 0}, {0, 50}, {50, 50}};

struct Enemy {
    int x, y;
    int speed;
};

vector<Enemy> enemies;
Uint32 lastSpawnTime = 0;

int player_x = SCREEN_WIDTH / 2;
int player_y = SCREEN_HEIGHT / 2;
int player_dx = 0, player_dy = 0;
int camera_x = 0;
int camera_y = 0;

struct Bullet {
    int x, y;
    int dx, dy;
};

vector<Bullet> bullets;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    window = SDL_CreateWindow("SDL2 Roguelike", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    return renderer != nullptr;
}

void spawnEnemy() {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastSpawnTime > ENEMY_SPAWN_INTERVAL) {
        int spawn_x = rand() % (DUNGEON_SIZE * TILE_SIZE * 2);
        int spawn_y = rand() % (DUNGEON_SIZE * TILE_SIZE * 2);
        enemies.push_back({spawn_x, spawn_y, 2});
        lastSpawnTime = currentTime;
    }
}

void update() {
    player_x += player_dx * SPEED;
    player_y += player_dy * SPEED;
    camera_x = player_x - SCREEN_WIDTH / 2;
    camera_y = player_y - SCREEN_HEIGHT / 2;

    for (auto& b : bullets) {
        b.x += b.dx;
        b.y += b.dy;
    }
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](Bullet& b) {
        return b.x < camera_x || b.x > camera_x + SCREEN_WIDTH ||
               b.y < camera_y || b.y > camera_y + SCREEN_HEIGHT;
    }), bullets.end());

    spawnEnemy();
    if (!isSpawnRoomRed && SDL_GetTicks() - spawnRoomChangeTime >= 20000) {
        isSpawnRoomRed = true;
    }

}

void updateEnemies() {
    for (auto& e : enemies) {
        int dx = player_x - e.x;
        int dy = player_y - e.y;
        float length = sqrt(dx * dx + dy * dy);
        if (length > 0) {
            e.x += e.speed * dx / length;
            e.y += e.speed * dy / length;
        }
    }
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [&](Bullet& b) {
        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
            int dx = b.x - it->x;
            int dy = b.y - it->y;
            if (dx * dx + dy * dy <= PLAYER_RADIUS * PLAYER_RADIUS) {
                enemies.erase(it);
                return true;
            }
        }
        return false;
    }), bullets.end());
}

void renderGrid() {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    for (int x = -camera_x % TILE_SIZE; x < SCREEN_WIDTH; x += TILE_SIZE) {
        SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_HEIGHT);
    }
    for (int y = -camera_y % TILE_SIZE; y < SCREEN_HEIGHT; y += TILE_SIZE) {
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_WIDTH, y);
    }
}

void renderMap() {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    for (auto& room : dungeons) {
        SDL_Rect border = {room.x * DUNGEON_SIZE * TILE_SIZE - camera_x,
                           room.y * DUNGEON_SIZE * TILE_SIZE - camera_y,
                           DUNGEON_SIZE * TILE_SIZE,
                           DUNGEON_SIZE * TILE_SIZE};
        SDL_RenderDrawRect(renderer, &border);
    }
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
for (auto& room : dungeons) {
    if (room.x == 0 && room.y == 0 && isSpawnRoomRed) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Đổi sang màu đỏ
    } else {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    }
    SDL_Rect border = {room.x * DUNGEON_SIZE * TILE_SIZE - camera_x,
                       room.y * DUNGEON_SIZE * TILE_SIZE - camera_y,
                       DUNGEON_SIZE * TILE_SIZE,
                       DUNGEON_SIZE * TILE_SIZE};
    SDL_RenderDrawRect(renderer, &border);
}
}

void renderCircle(int x, int y, int r) {
    for (int w = -r; w <= r; w++) {
        for (int h = -r; h <= r; h++) {
            if (w * w + h * h <= r * r) {
                SDL_RenderDrawPoint(renderer, x + w - camera_x, y + h - camera_y);
            }
        }
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    renderGrid();
    renderMap();
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    renderCircle(player_x, player_y, PLAYER_RADIUS);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (auto& e : enemies) {
        renderCircle(e.x, e.y, PLAYER_RADIUS);
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for (auto& b : bullets) {
        SDL_Rect rect = {b.x - camera_x, b.y - camera_y, BULLET_SIZE, BULLET_SIZE};
        SDL_RenderFillRect(renderer, &rect);
    }
    SDL_RenderPresent(renderer);
}

void handleInput(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
        int state = (e.type == SDL_KEYDOWN) ? 1 : 0;
        switch (e.key.keysym.sym) {
            case SDLK_w: player_dy = -state; break;
            case SDLK_s: player_dy = state; break;
            case SDLK_a: player_dx = -state; break;
            case SDLK_d: player_dx = state; break;
        }
    } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        mx += camera_x;
        my += camera_y;
        int dx = mx - player_x, dy = my - player_y;
        float length = sqrt(dx * dx + dy * dy);
        bullets.push_back({player_x, player_y, int(BULLET_SPEED * dx / length), int(BULLET_SPEED * dy / length)});
    }
}

int main(int argc, char* argv[]) {
    spawnRoomChangeTime = SDL_GetTicks();
    srand(time(0));
    if (!init()) return -1;
    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            handleInput(e);
        }
        update();
        updateEnemies();
        render();
        SDL_Delay(16);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
