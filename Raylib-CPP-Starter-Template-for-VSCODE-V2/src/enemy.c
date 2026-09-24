#include "enemy.h"
#include <math.h>

/* ============================================================
 *  Enemy — init / helpers
 * ============================================================ */

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

/* ============================================================
 *  Enemy — spawning
 * ============================================================ */

static bool Enemy_SpawnInternal(Enemy enemies[MAX_ENEMIES],
                                Rectangle platform, EnemyType type)
{
    float margin = ENEMY_WIDTH * 0.5f + 8.0f;
    if (platform.width < margin * 2.0f) return false;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            float usableW = platform.width - margin * 2.0f;
            float spawnX  = platform.x + margin +
                            (float)GetRandomValue(0, (int)usableW);

            enemies[i].type          = type;
            enemies[i].position      = (Vector2){ spawnX, platform.y };
            enemies[i].speed         = ENEMY_SPEED *
                                       (0.8f + (float)GetRandomValue(0, 40) / 100.0f);
            enemies[i].dir           = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;
            enemies[i].patrolLeft    = platform.x + margin;
            enemies[i].patrolRight   = platform.x + platform.width - margin;
            enemies[i].phase         = (float)GetRandomValue(0, 360) * DEG2RAD;
            enemies[i].stunTimer     = 0.0f;
            enemies[i].shootCooldown = ENEMY_SHOOT_COOLDOWN;
            enemies[i].active        = true;
            return true;
        }
    }
    return false;
}

bool Enemy_SpawnOnPlatform(Enemy enemies[MAX_ENEMIES], Rectangle platform) {
    return Enemy_SpawnInternal(enemies, platform, ENEMY_WALKER);
}

bool Enemy_SpawnShooterOnPlatform(Enemy enemies[MAX_ENEMIES], Rectangle platform) {
    return Enemy_SpawnInternal(enemies, platform, ENEMY_SHOOTER);
}

/* ============================================================
 *  Enemy — update
 * ============================================================ */

/* Fire a projectile travelling purely horizontally in `dir`. */
static void Enemy_FireProjectile(EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES],
                                 Vector2 origin, float dir)
{
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!projectiles[i].active) {
            projectiles[i].position = origin;
            projectiles[i].velocity = (Vector2){ dir * ENEMY_PROJECTILE_SPEED, 0.0f };
            projectiles[i].active   = true;
            return;
        }
    }
}

bool Enemy_UpdateAll(Enemy enemies[MAX_ENEMIES],
                     Vector2 playerPos, Rectangle playerRect,
                     EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES])
{
    bool  hit = false;
    float dt  = GetFrameTime();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        bool stunned = enemies[i].stunTimer > 0.0f;

        if (stunned) {
            enemies[i].stunTimer -= dt;
            if (enemies[i].stunTimer < 0.0f) enemies[i].stunTimer = 0.0f;
        } else {
            /* Walk */
            enemies[i].position.x += enemies[i].speed * enemies[i].dir;
            enemies[i].phase      += 0.10f;

            if (enemies[i].position.x <= enemies[i].patrolLeft) {
                enemies[i].position.x = enemies[i].patrolLeft;
                enemies[i].dir = 1.0f;
            } else if (enemies[i].position.x >= enemies[i].patrolRight) {
                enemies[i].position.x = enemies[i].patrolRight;
                enemies[i].dir = -1.0f;
            }

            /* Shooter behaviour (unchanged) */
            if (enemies[i].type == ENEMY_SHOOTER) {
                if (enemies[i].shootCooldown > 0.0f) {
                    enemies[i].shootCooldown -= dt;
                    if (enemies[i].shootCooldown < 0.0f) enemies[i].shootCooldown = 0.0f;
                }

                float dx = playerPos.x - enemies[i].position.x;
                float dy = playerPos.y - enemies[i].position.y;

                bool facingPlayer      = (dx > 0.0f && enemies[i].dir > 0.0f) ||
                                         (dx < 0.0f && enemies[i].dir < 0.0f);
                bool inHorizontalRange = fabsf(dx) < ENEMY_SHOOT_RANGE;
                bool onSameLevel       = fabsf(dy) < ENEMY_SHOOT_VERTICAL_TOLERANCE;

                if (facingPlayer && inHorizontalRange && onSameLevel &&
                    enemies[i].shootCooldown <= 0.0f)
                {
                    Vector2 origin = {
                        enemies[i].position.x + enemies[i].dir * 18.0f,
                        enemies[i].position.y - 22.0f
                    };
                    Enemy_FireProjectile(projectiles, origin, enemies[i].dir);
                    enemies[i].shootCooldown = ENEMY_SHOOT_COOLDOWN;
                }
            }

            /* Only ACTIVE (non-stunned) enemies hurt the player */
            if (CheckCollisionRecs(Enemy_GetRect(&enemies[i]), playerRect)) {
                hit = true;
            }
        }
        /* Stunned enemies: harmless — player can walk right past them */
    }

    return hit;
}

/* ============================================================
 *  Enemy — draw
 * ============================================================ */

void Enemy_DrawAll(const Enemy enemies[MAX_ENEMIES]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        bool  stunned = enemies[i].stunTimer > 0.0f;
        bool  shooter = enemies[i].type == ENEMY_SHOOTER;
        float x       = enemies[i].position.x;
        float baseY   = enemies[i].position.y;
        float facing  = enemies[i].dir;

        float bob   = stunned ? 0.0f : sinf(enemies[i].phase) * 1.5f;
        float swing = stunned ? 0.0f : sinf(enemies[i].phase * 2.0f) * 2.5f;

        Color body, dark, light;
        if (shooter) {
            body  = (Color){ 180,  60,  60, 255 };
            dark  = (Color){ 100,  25,  25, 255 };
            light = (Color){ 230, 120, 120, 255 };
        } else {
            body  = (Color){ 200, 105,  45, 255 };
            dark  = (Color){ 115,  50,  20, 255 };
            light = (Color){ 235, 160,  90, 255 };
        }
        Color eye = (Color){ 25, 10, 5, 255 };

        if (stunned) {
            body  = (Color){ 150, 170, 220, 255 };
            dark  = (Color){  70,  90, 140, 255 };
            light = (Color){ 200, 220, 255, 255 };
        }

        /* Stun aura */
        if (stunned) {
            float pulse = 0.6f + 0.4f * sinf((float)GetTime() * 12.0f);
            DrawCircleLines((int)x, (int)(baseY - 22), 22,
                            (Color){ 180, 220, 255, (unsigned char)(180 * pulse) });
            DrawCircle((int)(x - 20), (int)(baseY - 30), 2, (Color){ 220, 240, 255, 220 });
            DrawCircle((int)(x + 20), (int)(baseY - 15), 2, (Color){ 220, 240, 255, 220 });
            DrawCircle((int)x,        (int)(baseY - 45), 2, (Color){ 220, 240, 255, 220 });
        }

        /* Shadow */
        DrawEllipse((int)x, (int)(baseY + 1), 14, 4, (Color){ 0, 0, 0, 90 });

        /* Tail */
        DrawCircle((int)(x - facing * 15), (int)(baseY - 18 + bob), 3, dark);
        DrawCircle((int)(x - facing * 18), (int)(baseY - 24 + bob), 2, dark);

        /* Legs */
        DrawRectangle((int)(x - 8), (int)(baseY - 8), 5, (int)(8 + swing), dark);
        DrawRectangle((int)(x + 3), (int)(baseY - 8), 5, (int)(8 - swing), dark);

        /* Body */
        float bodyCY = baseY - 22 + bob;
        DrawEllipse((int)x, (int)bodyCY, 14, 14, body);
        DrawEllipseLines((int)x, (int)bodyCY, 14, 14, dark);
        DrawEllipse((int)(x + facing * 3), (int)(bodyCY + 3), 9, 8, light);

        /* Left arm */
        DrawCircle((int)(x - 13), (int)(bodyCY + 2), 4, dark);

        /* Right arm — extended cannon for shooters */
        if (shooter && !stunned) {
            DrawRectangle((int)(x + facing * 13 - 4),
                          (int)(bodyCY - 1), 8, 6, dark);
            DrawCircle((int)(x + facing * 19), (int)(bodyCY + 1), 4, dark);

            if (enemies[i].shootCooldown <= 0.0f) {
                float glow = 0.7f + 0.3f * sinf((float)GetTime() * 16.0f);
                DrawCircle((int)(x + facing * 21), (int)(bodyCY + 1),
                           3, (Color){ 255, 200, 100, (unsigned char)(220 * glow) });
                DrawCircle((int)(x + facing * 21), (int)(bodyCY + 1),
                           1, WHITE);
            }
        } else {
            DrawCircle((int)(x + 13), (int)(bodyCY + 2), 4, dark);
        }

        /* Shooter crest */
        if (shooter && !stunned) {
            DrawTriangle(
                (Vector2){ x - 5, bodyCY - 13 },
                (Vector2){ x,     bodyCY - 23 },
                (Vector2){ x + 5, bodyCY - 13 },
                dark
            );
        }

        /* Eyes */
        float eyeX = facing * 3.0f;
        DrawCircle((int)(x - 4 + eyeX), (int)(bodyCY - 3), 3, eye);
        DrawCircle((int)(x + 5 + eyeX), (int)(bodyCY - 3), 3, eye);
        DrawCircle((int)(x - 3 + eyeX), (int)(bodyCY - 4), 1, WHITE);
        DrawCircle((int)(x + 6 + eyeX), (int)(bodyCY - 4), 1, WHITE);
    }
}

/* ============================================================
 *  Enemy projectile — init / update / draw
 * ============================================================ */

void EnemyProjectile_InitAll(EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES]) {
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        projectiles[i].active = false;
    }
}

bool EnemyProjectile_UpdateAll(EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES],
                               Rectangle playerRect,
                               int screenWidth, int screenHeight)
{
    bool hit = false;

    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!projectiles[i].active) continue;

        projectiles[i].position.x += projectiles[i].velocity.x;
        projectiles[i].position.y += projectiles[i].velocity.y;

        if (projectiles[i].position.x < -50.0f ||
            projectiles[i].position.x > screenWidth + 50.0f ||
            projectiles[i].position.y < -50.0f ||
            projectiles[i].position.y > screenHeight + 50.0f)
        {
            projectiles[i].active = false;
            continue;
        }

        if (CheckCollisionCircleRec(projectiles[i].position,
                                    ENEMY_PROJECTILE_RADIUS, playerRect))
        {
            hit = true;
            projectiles[i].active = false;
        }
    }

    return hit;
}

void EnemyProjectile_DrawAll(const EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES]) {
    float t = (float)GetTime();

    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
        if (!projectiles[i].active) continue;

        Vector2 p     = projectiles[i].position;
        float   pulse = 0.85f + 0.15f * sinf(t * 22.0f);
        float   r     = ENEMY_PROJECTILE_RADIUS * pulse;

        DrawCircle((int)p.x, (int)p.y, r + 6, (Color){ 255, 120,  40,  70 });
        DrawCircle((int)p.x, (int)p.y, r,     (Color){ 255, 150,  60, 255 });
        DrawCircle((int)p.x, (int)p.y, r * 0.55f, (Color){ 255, 230, 160, 255 });
        DrawCircle((int)p.x, (int)p.y, r * 0.25f, WHITE);
    }
}