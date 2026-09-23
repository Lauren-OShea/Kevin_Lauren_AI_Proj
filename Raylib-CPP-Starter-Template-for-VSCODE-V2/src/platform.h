#ifndef JACKFROST_PLATFORM_H
#define JACKFROST_PLATFORM_H

#include <stdbool.h>
#include "raylib.h"

#define MAX_PLATFORMS 32
#define TILE_SIZE     40

typedef struct Platform {
    Rectangle bounds;     // world-space rect
    bool      active;
} Platform;

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