#ifndef SHARD_H
#define SHARD_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_SHARDS 32
#define SHARD_SPEED 6.0f
#define SHARD_RADIUS 10.0f

typedef struct Shard {
    Vector2 position;
    Vector2 velocity;
    bool active;
} Shard;

// Initialize all shards to inactive
void Shard_InitAll(Shard shards[MAX_SHARDS]);

// Fire a new shard from the given origin
void Shard_Shoot(Shard shards[MAX_SHARDS], Vector2 origin);

// Update all active shards and deactivate off-screen ones
void Shard_UpdateAll(Shard shards[MAX_SHARDS], int screenWidth);

// Draw all active shards
void Shard_DrawAll(const Shard shards[MAX_SHARDS]);

#endif // SHARD_H