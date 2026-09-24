#ifndef SHARD_H
#define SHARD_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_SHARDS   64
#define SHARD_SPEED  6.0f
#define SHARD_RADIUS 10.0f

typedef struct Shard {
    Vector2 position;
    Vector2 velocity;
    bool    active;
} Shard;

void Shard_InitAll  (Shard shards[MAX_SHARDS]);
void Shard_Shoot    (Shard shards[MAX_SHARDS], Vector2 origin, float dir);
void Shard_UpdateAll(Shard shards[MAX_SHARDS], int screenWidth);
void Shard_DrawAll  (const Shard shards[MAX_SHARDS]);

#endif /* SHARD_H */