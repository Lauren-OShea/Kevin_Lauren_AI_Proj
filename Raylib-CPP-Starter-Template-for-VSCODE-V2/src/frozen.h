#ifndef FROZEN_H
#define FROZEN_H

#include <stdbool.h>
#include "raylib.h"

#define FROZEN_TILE_SIZE   8
#define FROZEN_COLS        (800 / FROZEN_TILE_SIZE)
#define FROZEN_ROWS        (500 / FROZEN_TILE_SIZE)

/* How many tiles around the player get frozen each frame. */
#define FROZEN_RADIUS_TILES 2

typedef struct FrozenMap {
    unsigned char cells[FROZEN_ROWS][FROZEN_COLS];  /* 0 = not frozen, 1 = frozen */
} FrozenMap;

void Frozen_Init(FrozenMap *map);
void Frozen_Clear(FrozenMap *map);
void Frozen_FreezeCircle(FrozenMap *map, Vector2 center, float radiusPx);
bool Frozen_IsCellFrozen(const FrozenMap *map, int col, int row);
bool Frozen_IsRectFrozen(const FrozenMap *map, Rectangle rect);

/* Draw a frosted overlay for any frozen cell that overlaps `rect`. */
void Frozen_DrawOverlayRect(const FrozenMap *map, Rectangle rect,
                            Color frostColor);

#endif /* FROZEN_H */