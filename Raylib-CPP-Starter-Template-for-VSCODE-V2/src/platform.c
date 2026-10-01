#include "platform.h"
#include "frozen.h"

/* ============================================================
 *  Platforms
 * ============================================================ */

void Platform_InitAll(Platform platforms[MAX_PLATFORMS]) {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        platforms[i].active = false;
    }
}

int Platform_Add(Platform platforms[MAX_PLATFORMS], Rectangle bounds) {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].active) {
            platforms[i].bounds = bounds;
            platforms[i].active = true;
            return i;
        }
    }
    return -1;
}

void Platform_DrawAll(const Platform platforms[MAX_PLATFORMS],
                      const FrozenMap *frozen) {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].active) continue;

        Rectangle b = platforms[i].bounds;

        /* Figure out which grid cells this platform overlaps */
        int c0 = (int)(b.x) / FROZEN_TILE_SIZE;
        int c1 = (int)(b.x + b.width  - 1) / FROZEN_TILE_SIZE;
        int r0 = (int)(b.y) / FROZEN_TILE_SIZE;
        int r1 = (int)(b.y + b.height - 1) / FROZEN_TILE_SIZE;

        /* Draw each tile individually */
        for (int r = r0; r <= r1; r++) {
            for (int c = c0; c <= c1; c++) {
                Rectangle cell = {
                    (float)(c * FROZEN_TILE_SIZE),
                    (float)(r * FROZEN_TILE_SIZE),
                    (float)FROZEN_TILE_SIZE,
                    (float)FROZEN_TILE_SIZE
                };

                Rectangle tile = GetCollisionRec(cell, b);
                if (tile.width <= 0.0f || tile.height <= 0.0f) continue;

                bool isFrozen = frozen &&
                                Frozen_IsCellFrozen(frozen, c, r);

                Color body = isFrozen
                    ? (Color){ 170, 220, 255, 255 }   /* icy blue */
                    : (Color){ 200,  60,  60, 255 };  /* red brick */

                Color cap = isFrozen
                    ? (Color){ 220, 240, 255, 255 }
                    : (Color){ 240, 110, 110, 255 };

                Color line = isFrozen
                    ? (Color){ 100, 160, 210, 255 }
                    : (Color){ 130,  30,  30, 255 };

                DrawRectangleRec(tile, body);

                /* Snow cap only on top edge of the platform */
                if (tile.y == b.y) {
                    DrawRectangle((int)tile.x, (int)tile.y,
                                  (int)tile.width, 4, cap);
                }

                /* Outline every tile so they look like blocks */
                DrawRectangleLinesEx(tile, 1.0f, line);
            }
        }
    }
}

bool Platform_ContainsPoint(const Platform platforms[MAX_PLATFORMS], Vector2 point) {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].active) continue;
        if (CheckCollisionPointRec(point, platforms[i].bounds)) return true;
    }
    return false;
}

bool Platform_OverlapsRect(const Platform platforms[MAX_PLATFORMS], Rectangle rect) {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].active) continue;
        if (CheckCollisionRecs(rect, platforms[i].bounds)) return true;
    }
    return false;
}

/* ============================================================
 *  Ladders
 * ============================================================ */

void Ladder_InitAll(Ladder *ladders) {
    for (int i = 0; i < MAX_LADDERS; i++) {
        ladders[i].active = false;
    }
}

bool Ladder_Add(Ladder *ladders, Rectangle bounds) {
    for (int i = 0; i < MAX_LADDERS; i++) {
        if (!ladders[i].active) {
            ladders[i].bounds = bounds;
            ladders[i].active = true;
            return true;
        }
    }
    return false;
}

void Ladder_DrawAll(const Ladder *ladders,
                    const FrozenMap *frozen) {
    for (int i = 0; i < MAX_LADDERS; i++) {
        if (!ladders[i].active) continue;

        Rectangle b = ladders[i].bounds;

        int r0 = (int)(b.y) / FROZEN_TILE_SIZE;
        int r1 = (int)(b.y + b.height - 1) / FROZEN_TILE_SIZE;

        /* Draw each vertical tile of the ladder separately */
        for (int r = r0; r <= r1; r++) {
            Rectangle cell = {
                b.x,
                (float)(r * FROZEN_TILE_SIZE),
                b.width,
                (float)FROZEN_TILE_SIZE
            };

            Rectangle tile = GetCollisionRec(cell, b);
            if (tile.width <= 0.0f || tile.height <= 0.0f) continue;

            int c = (int)((b.x + b.width * 0.5f) / FROZEN_TILE_SIZE);
            bool isFrozen = frozen &&
                            Frozen_IsCellFrozen(frozen, c, r);

            Color rail = isFrozen
                ? (Color){ 100, 180, 240, 255 }
                : (Color){ 150,  90,  40, 255 };

            Color rung = isFrozen
                ? (Color){ 140, 210, 255, 255 }
                : (Color){ 180, 120,  60, 255 };

            /* Rails */
            DrawRectangle((int)tile.x, (int)tile.y,
                          2, (int)tile.height, rail);
            DrawRectangle((int)(tile.x + tile.width - 2), (int)tile.y,
                          2, (int)tile.height, rail);

            /* Rungs */
            for (float y = tile.y + 6; y < tile.y + tile.height - 2; y += 14.0f) {
                DrawRectangle((int)tile.x, (int)y,
                              (int)tile.width, 2, rung);
            }
        }
    }
}