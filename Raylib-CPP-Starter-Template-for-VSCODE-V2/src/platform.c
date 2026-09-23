#include "platform.h"

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

void Platform_DrawAll(const Platform platforms[MAX_PLATFORMS]) {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].active) continue;

        Rectangle b = platforms[i].bounds;

        // --- Main block body (brick-red) ---
        DrawRectangleRec(b, (Color){ 200,  40,  40, 255 });

        // --- Inner highlight (top edge) ---
        DrawRectangle((int)b.x, (int)b.y, (int)b.width, 6,
                      (Color){ 255, 120,  90, 255 });

        // --- Shadow (bottom edge) ---
        DrawRectangle((int)b.x, (int)(b.y + b.height - 6),
                      (int)b.width, 6, (Color){ 120,  20,  20, 255 });

        // --- Tile separators (vertical grid) ---
        for (int x = (int)b.x + TILE_SIZE; x < (int)(b.x + b.width); x += TILE_SIZE) {
            DrawRectangle(x - 1, (int)b.y, 2, (int)b.height,
                          (Color){ 140,  25,  25, 255 });
        }

        // --- Sparkle dots (icy accents on top) ---
        for (int x = (int)b.x + 10; x < (int)(b.x + b.width) - 10; x += 32) {
            DrawRectangle(x, (int)b.y + 3, 3, 3,
                          (Color){ 255, 200, 200, 180 });
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