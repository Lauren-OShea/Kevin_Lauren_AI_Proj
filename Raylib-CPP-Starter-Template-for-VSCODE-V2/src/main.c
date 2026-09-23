#include "raylib.h"

// Some Defines
#define PLAYER_MAX_LIFE 5
#define GRAVITY         2000.0f
#define MOVE_SPEED      250.0f
#define JUMP_SPEED      -700.0f
#define MAX_PLATFORMS   32

// Types and Structures
typedef struct Player {
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    int life;
    bool onGround;
    int facing;   // -1 = left, 1 = right
} Player;

typedef struct Platform {
    Rectangle rect;
} Platform;

// Global Variables
static const int screenWidth  = 1200;
static const int screenHeight = 800;

static Player player = {0};

static Platform platforms[MAX_PLATFORMS];
static int platformCount = 0;

// Module Functions
static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);

static Rectangle PlayerRect(void) {
    return (Rectangle){ player.position.x, player.position.y,
                        player.size.x, player.size.y };
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Basic Window!");
    InitGame();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }

    UnloadGame();
    CloseWindow();

    return 0;
}

// Initialize game variables
void InitGame(void) {
    player = (Player){
        .position = { 100, 100 },
        .size     = { 40, 60 },
        .velocity = { 0, 0 },
        .life     = PLAYER_MAX_LIFE,
        .onGround = false,
        .facing   = 1
    };

    platformCount = 0;

    // Ground (full width, at the bottom)
    platforms[platformCount++] = (Platform){ {   0, 750, 1200, 50 } };

    // --- Multi-level platforms scattered around ---
    // Left staircase going up
    platforms[platformCount++] = (Platform){ { 150, 650, 200, 20 } };
    platforms[platformCount++] = (Platform){ { 320, 550, 180, 20 } };
    platforms[platformCount++] = (Platform){ { 100, 440, 180, 20 } };

    // Middle pillar + floating islands
    platforms[platformCount++] = (Platform){ { 520, 600, 160, 20 } };
    platforms[platformCount++] = (Platform){ { 600, 430, 220, 20 } };
    platforms[platformCount++] = (Platform){ { 480, 280, 180, 20 } };

    // Right side
    platforms[platformCount++] = (Platform){ { 880, 620, 200, 20 } };
    platforms[platformCount++] = (Platform){ { 950, 470, 220, 20 } };
    platforms[platformCount++] = (Platform){ { 820, 340, 160, 20 } };
    platforms[platformCount++] = (Platform){ {1000, 200, 180, 20 } };

    // A wall on the far right (tall rectangle)
    platforms[platformCount++] = (Platform){ {1180, 500, 20, 250 } };
}

// ----- Horizontal collision: move, then push back out of anything we hit -----
static void MovePlayerX(float dx) {
    player.position.x += dx;
    Rectangle pr = PlayerRect();

    for (int i = 0; i < platformCount; i++) {
        Rectangle pl = platforms[i].rect;
        if (CheckCollisionRecs(pr, pl)) {
            if (dx > 0) {
                player.position.x = pl.x - player.size.x;
            } else if (dx < 0) {
                player.position.x = pl.x + pl.width;
            }
            pr = PlayerRect();
        }
    }

    // Clamp to screen
    if (player.position.x < 0) player.position.x = 0;
    if (player.position.x + player.size.x > screenWidth)
        player.position.x = screenWidth - player.size.x;
}

// ----- Vertical collision: same idea, but also sets onGround -----
static void MovePlayerY(float dy) {
    player.position.y += dy;
    player.onGround = false;
    Rectangle pr = PlayerRect();

    for (int i = 0; i < platformCount; i++) {
        Rectangle pl = platforms[i].rect;
        if (CheckCollisionRecs(pr, pl)) {
            if (dy > 0) {
                // Falling: land on top
                player.position.y = pl.y - player.size.y;
                player.velocity.y = 0;
                player.onGround = true;
            } else if (dy < 0) {
                // Moving up: bonk head
                player.position.y = pl.y + pl.height;
                player.velocity.y = 0;
            }
            pr = PlayerRect();
        }
    }
}

// Update game (one frame)
void UpdateGame(void) {
    float dt = GetFrameTime();

    // ----- Horizontal input (A / D) -----
    float dx = 0;
    if (IsKeyDown(KEY_A)) { dx -= MOVE_SPEED; player.facing = -1; }
    if (IsKeyDown(KEY_D)) { dx += MOVE_SPEED; player.facing =  1; }
    MovePlayerX(dx * dt);

    // ----- Jump (W) -----
    if (player.onGround && IsKeyPressed(KEY_W)) {
        player.velocity.y = JUMP_SPEED;
        player.onGround = false;
    }

    // ----- Gravity + vertical movement -----
    player.velocity.y += GRAVITY * dt;
    MovePlayerY(player.velocity.y * dt);

    // Fell off the world? Lose a life and respawn
    if (player.position.y > screenHeight + 200) {
        player.position = (Vector2){ 100, 100 };
        player.velocity = (Vector2){ 0, 0 };
        player.life--;
    }
}

// Draw game (one frame)
void DrawGame(void) {
    BeginDrawing();
    ClearBackground((Color){ 135, 206, 235, 255 });  // sky blue

    // Platforms
    for (int i = 0; i < platformCount; i++) {
        DrawRectangleRec(platforms[i].rect, DARKGREEN);
        // A slightly lighter top edge to make platforms readable
        DrawRectangle((int)platforms[i].rect.x,
                      (int)platforms[i].rect.y,
                      (int)platforms[i].rect.width,
                      3, LIME);
    }

    // Player
    DrawRectangleV(player.position, player.size, BLUE);

    // Eye to show facing
    float eyeX = player.position.x + (player.facing > 0
                    ? player.size.x * 0.65f
                    : player.size.x * 0.2f);
    DrawCircle((int)eyeX,
               (int)(player.position.y + player.size.y * 0.3f),
               4, WHITE);

    // HUD
    DrawText(TextFormat("Life: %d", player.life), 20, 20, 20, BLACK);
    DrawText("A/D move   W jump", 20, 45, 20, DARKGRAY);

    EndDrawing();
}

// Unload game variables
void UnloadGame(void) {
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void) {
    UpdateGame();
    DrawGame();
}