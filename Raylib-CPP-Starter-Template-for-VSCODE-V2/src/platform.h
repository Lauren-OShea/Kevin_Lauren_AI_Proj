#ifndef JACKFROST_PLATFORM_H
#define JACKFROST_PLATFORM_H

#include <stdbool.h>
#include "raylib.h"

#define MAX_PLATFORMS 32
#define TILE_SIZE     40
#define MAX_LADDERS         8
#define LADDER_WIDTH        16.0f
#define LADDER_CLIMB_SPEED  120.0f

typedef struct Platform {
    Rectangle bounds;     // world-space rect
    bool      active;
} Platform;


typedef struct Ladder {
    Rectangle bounds;
    bool      active;
} Ladder;

void Ladder_InitAll(Ladder *ladders);
bool Ladder_Add(Ladder *ladders, Rectangle bounds);
void Ladder_DrawAll(const Ladder *ladders);

// Initialize all platforms to inactive
void Platform_InitAll(Platform platforms[MAX_PLATFORMS]);

// Add a platform (returns index or -1 if full)
int  Platform_Add(Platform platforms[MAX_PLATFORMS], Rectangle bounds);

// Draw all active platforms
void Platform_DrawAll(const Platform platforms[MAX_PLATFORMS]);

// Returns true if the given world point is inside any active platform
bool Platform_ContainsPoint(const Platform platforms[MAX_PLATFORMS], Vector2 point);

// Returns true if the given rect overlaps any active platform
bool Platform_OverlapsRect(const Platform platforms[MAX_PLATFORMS], Rectangle rect);

#endif // JACKFROST_PLATFORM_H