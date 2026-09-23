#include "enemy.h"
#include <math.h>

void Enemy_InitAll(Enemy enemies[MAX_ENEMIES]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
}

void Enemy_Spawn(Enemy enemies[MAX_ENEMIES], int screenWidth, int screenHeight) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].position = (Vector2){
                screenWidth + 40,
                50 + (float)GetRandomValue(0, screenHeight - 100)
            };
            enemies[i].speed = ENEMY_BASE_SPEED * (0.7f + (float)GetRandomValue(0, 60) / 100.0f);
            enemies[i].phase = (float)GetRandomValue(0, 360) * DEG2RAD;
            enemies[i].active = true;
            return;
        }
    }
}

bool Enemy_UpdateAll(Enemy enemies[MAX_ENEMIES], Vector2 playerPos, float playerRadius) {
    bool playerHit = false;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        enemies[i].position.x -= enemies[i].speed;
        enemies[i].phase += 0.1f;

        // Collision with player
        float dx = enemies[i].position.x - playerPos.x;
        float dy = enemies[i].position.y - playerPos.y;
        if (sqrtf(dx * dx + dy * dy) < ENEMY_RADIUS + playerRadius) {
            playerHit = true;
        }

        // Off-screen cleanup
        if (enemies[i].position.x < -50) {
            enemies[i].active = false;
        }
    }

    return playerHit;
}

void Enemy_DrawAll(const Enemy enemies[MAX_ENEMIES]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        Vector2 p = enemies[i].position;
        float pulse = sinf(enemies[i].phase) * 2.0f;
        float r = ENEMY_RADIUS + pulse * 0.5f;

        // Fire glow
        DrawCircle(p.x, p.y, r + 6, (Color){ 255, 85, 0, 60 });
        DrawCircle(p.x, p.y, r, (Color){ 240, 107, 44, 255 });

        // Core
        DrawCircle(p.x - 3, p.y - 4, r * 0.6f, (Color){ 255, 179, 71, 255 });

        // Eyes
        DrawCircle(p.x - 8, p.y - 8, 5, (Color){ 30, 30, 30, 255 });
        DrawCircle(p.x + 8, p.y - 8, 5, (Color){ 30, 30, 30, 255 });
        DrawCircle(p.x - 8, p.y - 8, 2, WHITE);
        DrawCircle(p.x + 8, p.y - 8, 2, WHITE);
    }
}