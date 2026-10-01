#include "enemy.h"
#include <math.h>

/* ============================================================
 *  Enemy — init / helpers
 * ============================================================ */

void Enemy_InitAll(Enemy enemies[MAX_ENEMIES]) {
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
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
    if (!enemy->active)                    return false;
    if (enemy->type == ENEMY_UNSTUNNABLE)  return false;
    if (enemy->stunTimer > 0.0f)           return false;
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
            enemies[i].dir           = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;
            enemies[i].patrolLeft    = platform.x + margin;
            enemies[i].patrolRight   = platform.x + platform.width - margin;
            enemies[i].phase         = (float)GetRandomValue(0, 360) * DEG2RAD;
            enemies[i].flyPhase      = (float)GetRandomValue(0, 360) * DEG2RAD;
            enemies[i].stunTimer     = 0.0f;
            enemies[i].shootCooldown = ENEMY_SHOOT_COOLDOWN;
            enemies[i].dashActive    = 0.0f;
            enemies[i].dashTimer     = ENEMY_DASH_COOLDOWN *
                                       (0.5f + (float)GetRandomValue(0, 50) / 100.0f);
            enemies[i].baseY         = platform.y;
            enemies[i].velocityY     = 0.0f;
            enemies[i].airborne      = false;

            switch (type) {
                case ENEMY_FLYER:
                    enemies[i].position = (Vector2){
                        spawnX, platform.y - ENEMY_FLY_SPAWN_UP
                    };
                    enemies[i].baseY = enemies[i].position.y;
                    enemies[i].speed = ENEMY_SPEED *
                                       (0.7f + (float)GetRandomValue(0, 30) / 100.0f);
                    break;

                case ENEMY_UNSTUNNABLE:
                    enemies[i].position = (Vector2){ spawnX, platform.y };
                    enemies[i].speed    = ENEMY_UNSTUNNABLE_SPEED;
                    break;

                case ENEMY_DASHER:
                    enemies[i].position = (Vector2){ spawnX, platform.y };
                    enemies[i].speed    = ENEMY_SPEED *
                                          (0.7f + (float)GetRandomValue(0, 30) / 100.0f);
                    break;

                default:
                    enemies[i].position = (Vector2){ spawnX, platform.y };
                    enemies[i].speed    = ENEMY_SPEED *
                                          (0.8f + (float)GetRandomValue(0, 40) / 100.0f);
                    break;
            }

            enemies[i].active = true;
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
bool Enemy_SpawnDasherOnPlatform(Enemy enemies[MAX_ENEMIES], Rectangle platform) {
    return Enemy_SpawnInternal(enemies, platform, ENEMY_DASHER);
}
bool Enemy_SpawnUnstunnableOnPlatform(Enemy enemies[MAX_ENEMIES], Rectangle platform) {
    return Enemy_SpawnInternal(enemies, platform, ENEMY_UNSTUNNABLE);
}
bool Enemy_SpawnFlyer(Enemy enemies[MAX_ENEMIES], Rectangle platform) {
    return Enemy_SpawnInternal(enemies, platform, ENEMY_FLYER);
}

bool Enemy_PlaceDirect(Enemy enemies[MAX_ENEMIES], Vector2 position,
                       float baseY, float patrolLeft, float patrolRight,
                       EnemyType type)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].type          = type;
            enemies[i].position      = position;
            enemies[i].baseY         = baseY;
            enemies[i].patrolLeft    = patrolLeft;
            enemies[i].patrolRight   = patrolRight;
            enemies[i].speed         = (type == ENEMY_UNSTUNNABLE)
                                       ? ENEMY_UNSTUNNABLE_SPEED
                                       : ENEMY_SPEED;
            enemies[i].dir           = 1.0f;
            enemies[i].phase         = 0.0f;
            enemies[i].flyPhase      = 0.0f;
            enemies[i].stunTimer     = 0.0f;
            enemies[i].shootCooldown = ENEMY_SHOOT_COOLDOWN;
            enemies[i].dashActive    = 0.0f;
            enemies[i].dashTimer     = ENEMY_DASH_COOLDOWN;
            enemies[i].velocityY     = 0.0f;
            enemies[i].airborne      = false;
            enemies[i].active        = true;
            return true;
        }
    }
    return false;
}
/* ============================================================
 *  Enemy — update
 * ============================================================ */

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

static int NearestPlayerIndex(Vector2 from, Vector2 players[], int count) {
    int best = 0;
    float bestD2 = 1e9f;
    for (int i = 0; i < count; i++) {
        float dx = players[i].x - from.x;
        float dy = players[i].y - from.y;
        float d2 = dx * dx + dy * dy;
        if (d2 < bestD2) { bestD2 = d2; best = i; }
    }
    return best;
}

bool Enemy_UpdateAll(Enemy enemies[MAX_ENEMIES],
                     Vector2 playerPositions[],
                     Rectangle playerHitboxes[],
                     int playerCount,
                     EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES])
{
    bool  hit = false;
    float dt  = GetFrameTime();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        /* ---- Stunned: frozen, harmless, doesn't shoot ---- */
        if (enemies[i].stunTimer > 0.0f) {
            enemies[i].stunTimer -= dt;
            if (enemies[i].stunTimer < 0.0f) enemies[i].stunTimer = 0.0f;
            continue;
        }

        /* ---- Airborne: launched by a jumppad ---- */
        if (enemies[i].airborne) {
            enemies[i].velocityY += 0.7f;
            enemies[i].position.y += enemies[i].velocityY;

            if (enemies[i].velocityY > 0.0f &&
                enemies[i].position.y >= enemies[i].baseY) {
                enemies[i].position.y = enemies[i].baseY;
                enemies[i].velocityY  = 0.0f;
                enemies[i].airborne   = false;
            }
            continue;
        }

        /* ---- Movement (per type) ---- */
        switch (enemies[i].type) {
            case ENEMY_DASHER: {
                if (enemies[i].dashActive > 0.0f) {
                    enemies[i].dashActive -= dt;
                    if (enemies[i].dashActive < 0.0f)
                        enemies[i].dashActive = 0.0f;
                } else {
                    enemies[i].dashTimer -= dt;
                    if (enemies[i].dashTimer <= 0.0f) {
                        enemies[i].dashActive = ENEMY_DASH_DURATION;
                        enemies[i].dashTimer  = ENEMY_DASH_COOLDOWN *
                                                (0.8f + (float)GetRandomValue(0, 60) / 100.0f);
                    }
                }
                float mult = (enemies[i].dashActive > 0.0f) ? ENEMY_DASH_MULT : 1.0f;
                enemies[i].position.x += enemies[i].speed * mult * enemies[i].dir;
            } break;

            case ENEMY_FLYER: {
                enemies[i].position.x += enemies[i].speed * enemies[i].dir;
                enemies[i].flyPhase   += ENEMY_FLY_SPEED;
                enemies[i].position.y  = enemies[i].baseY +
                                         sinf(enemies[i].flyPhase) * ENEMY_FLY_AMPLITUDE;
            } break;

            default:
                enemies[i].position.x += enemies[i].speed * enemies[i].dir;
                break;
        }

        enemies[i].phase += 0.10f;

        /* Bounce off patrol bounds */
        if (enemies[i].position.x <= enemies[i].patrolLeft) {
            enemies[i].position.x = enemies[i].patrolLeft;
            enemies[i].dir = 1.0f;
        } else if (enemies[i].position.x >= enemies[i].patrolRight) {
            enemies[i].position.x = enemies[i].patrolRight;
            enemies[i].dir = -1.0f;
        }

        /* ---- Shooter behaviour ---- */
        if (enemies[i].type == ENEMY_SHOOTER ||
            enemies[i].type == ENEMY_FLYER)
        {
            if (enemies[i].shootCooldown > 0.0f) {
                enemies[i].shootCooldown -= dt;
                if (enemies[i].shootCooldown < 0.0f)
                    enemies[i].shootCooldown = 0.0f;
            }

            int nearest = NearestPlayerIndex(enemies[i].position,
                                             playerPositions, playerCount);

            float dx = playerPositions[nearest].x - enemies[i].position.x;
            float dy = playerPositions[nearest].y - enemies[i].position.y;

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

        /* ---- Contact damage ---- */
        for (int p = 0; p < playerCount; p++) {
            if (CheckCollisionRecs(Enemy_GetRect(&enemies[i]),
                                   playerHitboxes[p])) {
                hit = true;
            }
        }
    }

    return hit;
}

/* ============================================================
 *  Enemy — draw
 * ============================================================ */

static void DrawWalkerOrShooter(const Enemy *e,
                                Color body, Color dark, Color light,
                                float x, float baseY,
                                float bob, float swing, float facing,
                                bool shooter)
{
    Color eye = (Color){ 25, 10, 5, 255 };

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

    if (shooter) {
        DrawRectangle((int)(x + facing * 13 - 4),
                      (int)(bodyCY - 1), 8, 6, dark);
        DrawCircle((int)(x + facing * 19), (int)(bodyCY + 1), 4, dark);

        if (e->shootCooldown <= 0.0f) {
            float glow = 0.7f + 0.3f * sinf((float)GetTime() * 16.0f);
            DrawCircle((int)(x + facing * 21), (int)(bodyCY + 1),
                       3, (Color){ 255, 200, 100, (unsigned char)(220 * glow) });
            DrawCircle((int)(x + facing * 21), (int)(bodyCY + 1), 1, WHITE);
        }

        DrawTriangle(
            (Vector2){ x - 5, bodyCY - 13 },
            (Vector2){ x,     bodyCY - 23 },
            (Vector2){ x + 5, bodyCY - 13 },
            dark
        );
    } else {
        DrawCircle((int)(x + 13), (int)(bodyCY + 2), 4, dark);
    }

    float eyeX = facing * 3.0f;
    DrawCircle((int)(x - 4 + eyeX), (int)(bodyCY - 3), 3, eye);
    DrawCircle((int)(x + 5 + eyeX), (int)(bodyCY - 3), 3, eye);
    DrawCircle((int)(x - 3 + eyeX), (int)(bodyCY - 4), 1, WHITE);
    DrawCircle((int)(x + 6 + eyeX), (int)(bodyCY - 4), 1, WHITE);
}

void Enemy_DrawAll(const Enemy enemies[MAX_ENEMIES]) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        bool  stunned = enemies[i].stunTimer > 0.0f;
        float x       = enemies[i].position.x;
        float baseY   = enemies[i].position.y;
        float facing  = enemies[i].dir;

        float bob   = stunned ? 0.0f : sinf(enemies[i].phase) * 1.5f;
        float swing = stunned ? 0.0f : sinf(enemies[i].phase * 2.0f) * 2.5f;

        /* Stun aura */
        if (stunned) {
            float pulse = 0.6f + 0.4f * sinf((float)GetTime() * 12.0f);
            DrawCircleLines((int)x, (int)(baseY - 22), 22,
                            (Color){ 180, 220, 255, (unsigned char)(180 * pulse) });
            DrawCircle((int)(x - 20), (int)(baseY - 30), 2, (Color){ 220, 240, 255, 220 });
            DrawCircle((int)(x + 20), (int)(baseY - 15), 2, (Color){ 220, 240, 255, 220 });
            DrawCircle((int)x,        (int)(baseY - 45), 2, (Color){ 220, 240, 255, 220 });
        }

        switch (enemies[i].type) {

        case ENEMY_WALKER: {
            Color body  = (Color){ 200, 105,  45, 255 };
            Color dark  = (Color){ 115,  50,  20, 255 };
            Color light = (Color){ 235, 160,  90, 255 };
            if (stunned) {
                body  = (Color){ 150, 170, 220, 255 };
                dark  = (Color){  70,  90, 140, 255 };
                light = (Color){ 200, 220, 255, 255 };
            }
            DrawWalkerOrShooter(&enemies[i], body, dark, light,
                                x, baseY, bob, swing, facing, false);
        } break;

        case ENEMY_SHOOTER: {
            Color body  = (Color){ 180,  60,  60, 255 };
            Color dark  = (Color){ 100,  25,  25, 255 };
            Color light = (Color){ 230, 120, 120, 255 };
            if (stunned) {
                body  = (Color){ 150, 170, 220, 255 };
                dark  = (Color){  70,  90, 140, 255 };
                light = (Color){ 200, 220, 255, 255 };
            }
            DrawWalkerOrShooter(&enemies[i], body, dark, light,
                                x, baseY, bob, swing, facing, true);
        } break;

        case ENEMY_DASHER: {
            Color body  = (Color){ 230, 175,  50, 255 };
            Color dark  = (Color){ 130,  85,  15, 255 };
            Color light = (Color){ 255, 225, 140, 255 };
            if (stunned) {
                body  = (Color){ 150, 170, 220, 255 };
                dark  = (Color){  70,  90, 140, 255 };
                light = (Color){ 200, 220, 255, 255 };
            }

            if (enemies[i].dashActive > 0.0f && !stunned) {
                Color trail = (Color){ 255, 220, 120, 140 };
                for (int k = 1; k <= 3; k++) {
                    float tx = x - facing * (10.0f + k * 8.0f);
                    DrawRectangle((int)(tx - 6), (int)(baseY - 26),
                                  12, 3, trail);
                }
            }

            DrawWalkerOrShooter(&enemies[i], body, dark, light,
                                x, baseY, bob, swing, facing, false);

            if (!stunned) {
                float bodyCY = baseY - 22 + bob;
                DrawRectangle((int)(x - 14), (int)(bodyCY - 6),
                              28, 6, (Color){ 30, 30, 30, 255 });
                DrawRectangle((int)(x - 12), (int)(bodyCY - 5),
                              8, 4, (Color){ 120, 200, 255, 255 });
                DrawRectangle((int)(x + 4),  (int)(bodyCY - 5),
                              8, 4, (Color){ 120, 200, 255, 255 });
            }
        } break;

        case ENEMY_FLYER: {
            Color body  = (Color){ 130, 130, 220, 255 };
            Color dark  = (Color){  60,  60, 130, 255 };
            Color light = (Color){ 190, 200, 255, 255 };
            if (stunned) {
                body  = (Color){ 150, 170, 220, 255 };
                dark  = (Color){  70,  90, 140, 255 };
                light = (Color){ 200, 220, 255, 255 };
            }

            Color eye = (Color){ 20, 10, 40, 255 };

            DrawEllipse((int)x, (int)(baseY + ENEMY_FLY_SPAWN_UP + 1),
                        10, 3, (Color){ 0, 0, 0, 60 });

            float bodyCY = baseY - 18 + bob;

            float flap = sinf(enemies[i].flyPhase * 6.0f) * 0.5f + 0.5f;
            float wingTipL = x - 22 - flap * 4.0f;
            float wingTipR = x + 22 + flap * 4.0f;

            DrawTriangle(
                (Vector2){ x - 6, bodyCY },
                (Vector2){ wingTipL, bodyCY - 12 },
                (Vector2){ x - 8, bodyCY + 6 },
                (Color){ light.r, light.g, light.b, 200 }
            );
            DrawTriangle(
                (Vector2){ x + 6, bodyCY },
                (Vector2){ wingTipR, bodyCY - 12 },
                (Vector2){ x + 8, bodyCY + 6 },
                (Color){ light.r, light.g, light.b, 200 }
            );

            DrawEllipse((int)x, (int)bodyCY, 12, 14, body);
            DrawEllipseLines((int)x, (int)bodyCY, 12, 14, dark);

            if (!stunned) {
                DrawRectangle((int)(x + facing * 10 - 4),
                              (int)(bodyCY - 1), 8, 5, dark);
                DrawCircle((int)(x + facing * 15), (int)(bodyCY + 1), 3, dark);

                if (enemies[i].shootCooldown <= 0.0f) {
                    float glow = 0.7f + 0.3f * sinf((float)GetTime() * 16.0f);
                    DrawCircle((int)(x + facing * 17), (int)(bodyCY + 1),
                               3, (Color){ 255, 200, 100,
                                           (unsigned char)(220 * glow) });
                    DrawCircle((int)(x + facing * 17), (int)(bodyCY + 1),
                               1, WHITE);
                }
            }

            float eyeX = facing * 2.0f;
            DrawCircle((int)(x - 4 + eyeX), (int)(bodyCY - 3), 3, eye);
            DrawCircle((int)(x + 4 + eyeX), (int)(bodyCY - 3), 3, eye);
            DrawCircle((int)(x - 3 + eyeX), (int)(bodyCY - 4), 1, WHITE);
            DrawCircle((int)(x + 5 + eyeX), (int)(bodyCY - 4), 1, WHITE);
        } break;

        case ENEMY_UNSTUNNABLE: {
            Color body  = (Color){ 70, 70, 80, 255 };
            Color dark  = (Color){ 30, 30, 40, 255 };
            Color light = (Color){ 130, 130, 150, 255 };
            Color eye   = (Color){ 220, 60, 60, 255 };

            float swingU = sinf(enemies[i].phase * 2.0f) * 1.8f;

            DrawEllipse((int)x, (int)(baseY + 1), 16, 5, (Color){ 0, 0, 0, 110 });

            DrawRectangle((int)(x - 10), (int)(baseY - 8), 7,
                          (int)(8 + swingU), dark);
            DrawRectangle((int)(x + 3),  (int)(baseY - 8), 7,
                          (int)(8 - swingU), dark);

            float bodyCY = baseY - 22 + bob * 0.5f;
            DrawEllipse((int)x, (int)bodyCY, 17, 16, body);
            DrawEllipseLines((int)x, (int)bodyCY, 17, 16, dark);

            DrawRectangle((int)(x - 15), (int)(bodyCY - 2), 30, 4, light);
            DrawRectangle((int)(x - 12), (int)(bodyCY + 5), 24, 3, light);

            DrawTriangle(
                (Vector2){ x - 8, bodyCY - 14 },
                (Vector2){ x,     bodyCY - 26 },
                (Vector2){ x + 8, bodyCY - 14 },
                dark
            );

            float eyeX = facing * 3.0f;
            DrawCircle((int)(x - 5 + eyeX), (int)(bodyCY - 3), 3, eye);
            DrawCircle((int)(x + 5 + eyeX), (int)(bodyCY - 3), 3, eye);
            DrawCircle((int)(x - 5 + eyeX), (int)(bodyCY - 3), 1, WHITE);
            DrawCircle((int)(x + 5 + eyeX), (int)(bodyCY - 3), 1, WHITE);

            DrawCircleLines((int)(x + 14), (int)(bodyCY + 2), 5,
                            (Color){ 200, 200, 220, 255 });
            DrawLineEx((Vector2){ x + 14, bodyCY - 2 },
                       (Vector2){ x + 14, bodyCY + 6 },
                       2.0f, (Color){ 200, 200, 220, 255 });
        } break;
        }
    }
}

/* ============================================================
 *  Enemy projectile — init / update / draw
 * ============================================================ */

void EnemyProjectile_InitAll(EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES]) {
    for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) projectiles[i].active = false;
}

bool EnemyProjectile_UpdateAll(EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES],
                               Rectangle playerHitboxes[],
                               int playerCount,
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

        for (int p = 0; p < playerCount; p++) {
            if (CheckCollisionCircleRec(projectiles[i].position,
                                        ENEMY_PROJECTILE_RADIUS,
                                        playerHitboxes[p]))
            {
                hit = true;
                projectiles[i].active = false;
                break;
            }
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