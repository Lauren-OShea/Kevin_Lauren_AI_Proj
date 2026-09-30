#ifndef JACKFROST_ENEMY_H
#define JACKFROST_ENEMY_H

#include <stdbool.h>
#include "raylib.h"

#define MAX_ENEMIES      48
#define ENEMY_WIDTH      32
#define ENEMY_HEIGHT     36
#define ENEMY_SPEED      1.5f
#define ENEMY_STUN_TIME  6.5f

#define MAX_ENEMY_PROJECTILES           32
#define ENEMY_SHOOT_RANGE               160.0f
#define ENEMY_SHOOT_VERTICAL_TOLERANCE   40.0f
#define ENEMY_SHOOT_COOLDOWN              1.4f
#define ENEMY_PROJECTILE_SPEED            3.8f
#define ENEMY_PROJECTILE_RADIUS           6.0f

/* ---- Dasher ---- */
#define ENEMY_DASH_COOLDOWN  2.2f
#define ENEMY_DASH_DURATION  0.55f
#define ENEMY_DASH_MULT      3.4f

/* ---- Flyer ---- */
#define ENEMY_FLY_AMPLITUDE  10.0f
#define ENEMY_FLY_SPEED      0.06f
#define ENEMY_FLY_SPAWN_UP   95.0f

/* ---- Unstunnable ---- */
#define ENEMY_UNSTUNNABLE_SPEED  0.75f

typedef enum EnemyType {
    ENEMY_WALKER = 0,
    ENEMY_SHOOTER,
    ENEMY_DASHER,
    ENEMY_FLYER,
    ENEMY_UNSTUNNABLE
} EnemyType;

typedef struct Enemy {
    EnemyType type;
    Vector2   position;
    float     baseY;        /* flyer vertical anchor */
    float     speed;
    float     dir;
    float     patrolLeft;
    float     patrolRight;
    float     phase;        /* walk bob */
    float     flyPhase;     /* flyer float */
    float     stunTimer;
    float     shootCooldown;
    float     dashTimer;    /* dasher: seconds until next dash */
    float     dashActive;   /* dasher: seconds remaining in dash */
    bool      active;
} Enemy;

typedef struct EnemyProjectile {
    Vector2 position;
    Vector2 velocity;
    bool    active;
} EnemyProjectile;

void Enemy_InitAll(Enemy enemies[MAX_ENEMIES]);

bool Enemy_SpawnOnPlatform          (Enemy enemies[MAX_ENEMIES], Rectangle platform);
bool Enemy_SpawnShooterOnPlatform   (Enemy enemies[MAX_ENEMIES], Rectangle platform);
bool Enemy_SpawnDasherOnPlatform    (Enemy enemies[MAX_ENEMIES], Rectangle platform);
bool Enemy_SpawnUnstunnableOnPlatform(Enemy enemies[MAX_ENEMIES], Rectangle platform);
bool Enemy_SpawnFlyer               (Enemy enemies[MAX_ENEMIES], Rectangle platform);

bool Enemy_UpdateAll(Enemy enemies[MAX_ENEMIES],
                     Vector2 playerPositions[],
                     Rectangle playerHitboxes[],
                     int playerCount,
                     EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES]);

void Enemy_DrawAll(const Enemy enemies[MAX_ENEMIES]);

Rectangle Enemy_GetRect(const Enemy *enemy);
bool      Enemy_Stun   (Enemy *enemy);

void EnemyProjectile_InitAll(EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES]);

bool EnemyProjectile_UpdateAll(EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES],
                               Rectangle playerHitboxes[],
                               int playerCount,
                               int screenWidth, int screenHeight);

void EnemyProjectile_DrawAll(const EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES]);

#endif /* JACKFROST_ENEMY_H */