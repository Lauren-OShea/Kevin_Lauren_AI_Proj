#ifndef JACKFROST_FROZEN_H
#define JACKFROST_FROZEN_H

#include <stdbool.h>
#include "raylib.h"

#define FROZEN_TILE_SIZE    40
#define FROZEN_COLS         40
#define FROZEN_ROWS         20

#define FROZEN_RADIUS_TILES 3.0f

typedef struct FrozenMap {
    unsigned char cells[FROZEN_ROWS][FROZEN_COLS];
} FrozenMap;

void Frozen_Init(FrozenMap *map);
void Frozen_Clear(FrozenMap *map);
bool Frozen_IsCellFrozen(const FrozenMap *map, int col, int row);
bool Frozen_IsRectFrozen(const FrozenMap *map, Rectangle rect);
void Frozen_FreezeCircle(FrozenMap *map, Vector2 center, float radiusPx);
void Frozen_DrawOverlayRect(const FrozenMap *map, Rectangle rect, Color frostColor);
bool Frozen_IsFull(const FrozenMap *map);

#endif /* JACKFROST_FROZEN_H */