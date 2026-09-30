#include "frozen.h"
#include <math.h>

void Frozen_Init(FrozenMap *map) {
    Frozen_Clear(map);
}

void Frozen_Clear(FrozenMap *map) {
    for (int r = 0; r < FROZEN_ROWS; r++)
        for (int c = 0; c < FROZEN_COLS; c++)
            map->cells[r][c] = 0;
}

bool Frozen_IsCellFrozen(const FrozenMap *map, int col, int row) {
    if (col < 0 || col >= FROZEN_COLS) return false;
    if (row < 0 || row >= FROZEN_ROWS) return false;
    return map->cells[row][col] != 0;
}

bool Frozen_IsRectFrozen(const FrozenMap *map, Rectangle rect) {
    int c0 = (int)(rect.x) / FROZEN_TILE_SIZE;
    int c1 = (int)(rect.x + rect.width  - 1) / FROZEN_TILE_SIZE;
    int r0 = (int)(rect.y) / FROZEN_TILE_SIZE;
    int r1 = (int)(rect.y + rect.height - 1) / FROZEN_TILE_SIZE;

    for (int r = r0; r <= r1; r++)
        for (int c = c0; c <= c1; c++)
            if (Frozen_IsCellFrozen(map, c, r)) return true;

    return false;
}

/* Freeze every cell whose centre is within radiusPx of center. */
void Frozen_FreezeCircle(FrozenMap *map, Vector2 center, float radiusPx) {
    int c0 = (int)((center.x - radiusPx) / FROZEN_TILE_SIZE);
    int c1 = (int)((center.x + radiusPx) / FROZEN_TILE_SIZE);
    int r0 = (int)((center.y - radiusPx) / FROZEN_TILE_SIZE);
    int r1 = (int)((center.y + radiusPx) / FROZEN_TILE_SIZE);

    float r2 = radiusPx * radiusPx;

    if (c0 < 0) c0 = 0;
    if (r0 < 0) r0 = 0;
    if (c1 >= FROZEN_COLS) c1 = FROZEN_COLS - 1;
    if (r1 >= FROZEN_ROWS) r1 = FROZEN_ROWS - 1;

    for (int r = r0; r <= r1; r++) {
        for (int c = c0; c <= c1; c++) {
            float cx = (c + 0.5f) * FROZEN_TILE_SIZE;
            float cy = (r + 0.5f) * FROZEN_TILE_SIZE;
            float dx = cx - center.x;
            float dy = cy - center.y;
            if (dx * dx + dy * dy <= r2) {
                map->cells[r][c] = 1;
            }
        }
    }
}

/* Draw a translucent icy rectangle over every frozen cell that
   overlaps `rect`. This is called by Platform/Ladder drawing. */
void Frozen_DrawOverlayRect(const FrozenMap *map, Rectangle rect,
                            Color frostColor) {
    int c0 = (int)(rect.x) / FROZEN_TILE_SIZE;
    int c1 = (int)(rect.x + rect.width  - 1) / FROZEN_TILE_SIZE;
    int r0 = (int)(rect.y) / FROZEN_TILE_SIZE;
    int r1 = (int)(rect.y + rect.height - 1) / FROZEN_TILE_SIZE;

    if (c0 < 0) c0 = 0;
    if (r0 < 0) r0 = 0;
    if (c1 >= FROZEN_COLS) c1 = FROZEN_COLS - 1;
    if (r1 >= FROZEN_ROWS) r1 = FROZEN_ROWS - 1;

    for (int r = r0; r <= r1; r++) {
        for (int c = c0; c <= c1; c++) {
            if (map->cells[r][c] == 0) continue;

            Rectangle cell = {
                (float)(c * FROZEN_TILE_SIZE),
                (float)(r * FROZEN_TILE_SIZE),
                (float)FROZEN_TILE_SIZE,
                (float)FROZEN_TILE_SIZE
            };
            /* Clip to the platform/ladder rect so we don't draw
               outside the body. */
            Rectangle clip = GetCollisionRec(cell, rect);
            DrawRectangleRec(clip, frostColor);
        }
    }
}