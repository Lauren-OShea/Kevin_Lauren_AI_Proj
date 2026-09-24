#include "enemy.h"
#include <math.h>

void Enemy_InitAll(Enemy enemies[MAX_ENEMIES]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
}

Rectangle Enemy_GetRect(const Enemy *enemy) {
    return (Rectangle){
        enemy->position.x - ENEMY_WIDTH  * 0.5f,
        enemy->position.y - ENEMY_HEIGHT,
        (float)ENEMY_WIDTH,
        (float)ENEMY_HEIGHT
    };
}

bool Enemy_Stun(Enemy *enemy) {
    if (!enemy->active) return false;
    if (enemy->stunTimer > 0.0f) return false;
    enemy->stunTimer = ENEMY_STUN_TIME;
    return true;
}

bool Enemy_SpawnOnPlatform(Enemy enemies[MAX_ENEMIES], Rectangle platform) {
    float margin = ENEMY_WIDTH * 0.5f + 8.0f;
    if (platform.width < margin * 2.0f) return false;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            float usableW = platform.width - margin * 2.0f;
            float spawnX  = platform.x + margin +
                            (float)GetRandomValue(0, (int)usableW);

            enemies[i].position    = (Vector2){ spawnX, platform.y };
            enemies[i].speed       = ENEMY_SPEED *
                                     (0.8f + (float)GetRandomValue(0, 40) / 100.0f);
            enemies[i].dir         = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;
            enemies[i].patrolLeft  = platform.x + margin;
            enemies[i].patrolRight = platform.x + platform.width - margin;
            enemies[i].phase       = (float)GetRandomValue(0, 360) * DEG2RAD;
            enemies[i].stunTimer   = 0.0f;
            enemies[i].active      = true;
            return true;
        }
    }
    return false;
}

bool Enemy_UpdateAll(Enemy enemies[MAX_ENEMIES], Rectangle playerRect) {
    bool hit = false;
    float dt = GetFrameTime();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        // Tick down stun timer
        if (enemies[i].stunTimer > 0.0f) {
            enemies[i].stunTimer -= dt;
            if (enemies[i].stunTimer < 0.0f) enemies[i].stunTimer = 0.0f;
        }

        // Only move when not stunned
        if (enemies[i].stunTimer == 0.0f) {
            enemies[i].position.x += enemies[i].speed * enemies[i].dir;
            enemies[i].phase      += 0.10f;

            if (enemies[i].position.x <= enemies[i].patrolLeft) {
                enemies[i].position.x = enemies[i].patrolLeft;
                enemies[i].dir = 1.0f;
            } else if (enemies[i].position.x >= enemies[i].patrolRight) {
                enemies[i].position.x = enemies[i].patrolRight;
                enemies[i].dir = -1.0f;
            }
        }

        if (CheckCollisionRecs(Enemy_GetRect(&enemies[i]), playerRect)) {
            hit = true;
        }
    }

    return hit;
}

void Enemy_DrawAll(const Enemy enemies[MAX_ENEMIES]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        bool  stunned = enemies[i].stunTimer > 0.0f;
        float x       = enemies[i].position.x;
        float baseY   = enemies[i].position.y;
        float facing  = enemies[i].dir;

        // When stunned, freeze the walk cycle (no bob / no leg swing)
        float bob   = stunned ? 0.0f : sinf(enemies[i].phase) * 1.5f;
        float swing = stunned ? 0.0f : sinf(enemies[i].phase * 2.0f) * 2.5f;

        Color body  = (Color){ 200, 105,  45, 255 };
        Color dark  = (Color){ 115,  50,  20, 255 };
        Color light = (Color){ 235, 160,  90, 255 };
        Color eye   = (Color){  25,  10,   5, 255 };

        // Stunned tint: shift toward icy blue
        if (stunned) {
            body  = (Color){ 150, 170, 220, 255 };
            dark  = (Color){  70,  90, 140, 255 };
            light = (Color){ 200, 220, 255, 255 };
        }

        // Stun aura (icy ring + sparkles)
        if (stunned) {
            float pulse = 0.6f + 0.4f * sinf((float)GetTime() * 12.0f);
            DrawCircleLines((int)x, (int)(baseY - 22), 22,
                            (Color){ 180, 220, 255, (unsigned char)(180 * pulse) });

            DrawCircle((int)(x - 20), (int)(baseY - 30), 2,
                       (Color){ 220, 240, 255, 220 });
            DrawCircle((int)(x + 20), (int)(baseY - 15), 2,
                       (Color){ 220, 240, 255, 220 });
            DrawCircle((int)x, (int)(baseY - 45), 2,
                       (Color){ 220, 240, 255, 220 });
        }

        DrawEllipse((int)x, (int)(baseY + 1), 14, 4, (Color){ 0, 0, 0, 90 });

        DrawCircle((int)(x - facing * 15), (int)(baseY - 18 + bob), 3, dark);
        DrawCircle((int)(x - facing * 18), (int)(baseY - 24 + bob), 2, dark);

        DrawRectangle((int)(x - 8), (int)(baseY - 8), 5, (int)(8 + swing), dark);
        DrawRectangle((int)(x + 3), (int)(baseY - 8), 5, (int)(8 - swing), dark);

        float bodyCY = baseY - 22 + bob;
        DrawEllipse((int)x, (int)bodyCY, 14, 14, body);
        DrawEllipseLines((int)x, (int)bodyCY, 14, 14, dark);

        DrawEllipse((int)(x + facing * 3), (int)(bodyCY + 3), 9, 8, light);

        DrawCircle((int)(x - 13), (int)(bodyCY + 2), 4, dark);
        DrawCircle((int)(x + 13), (int)(bodyCY + 2), 4, dark);

        float eyeX = facing * 3.0f;
        DrawCircle((int)(x - 4 + eyeX), (int)(bodyCY - 3), 3, eye);
        DrawCircle((int)(x + 5 + eyeX), (int)(bodyCY - 3), 3, eye);
        DrawCircle((int)(x - 3 + eyeX), (int)(bodyCY - 4), 1, WHITE);
        DrawCircle((int)(x + 6 + eyeX), (int)(bodyCY - 4), 1, WHITE);
    }
}