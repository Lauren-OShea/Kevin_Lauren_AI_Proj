#include "platform.h"

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

void Platform_DrawAll(const Platform platforms[MAX_PLATFORMS]) {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].active) continue;

        Rectangle b = platforms[i].bounds;

        /* Body */
        DrawRectangleRec(b, (Color){ 90, 140, 200, 255 });

        /* Snow cap on top */
        DrawRectangle((int)b.x, (int)b.y,
                      (int)b.width, 4,
                      (Color){ 240, 250, 255, 255 });

        /* Subtle outline */
        DrawRectangleLinesEx(b, 1.0f, (Color){ 40, 80, 130, 255 });
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

void Ladder_DrawAll(const Ladder *ladders) {
    for (int i = 0; i < MAX_LADDERS; i++) {
        if (!ladders[i].active) continue;

        Rectangle b = ladders[i].bounds;

        /* Two vertical rails */
        DrawRectangle((int)b.x, (int)b.y,
                      2, (int)b.height, (Color){ 150, 90, 40, 255 });
        DrawRectangle((int)(b.x + b.width - 2), (int)b.y,
                      2, (int)b.height, (Color){ 150, 90, 40, 255 });

        /* Rungs every 14 px */
        for (float y = b.y + 6; y < b.y + b.height; y += 14.0f) {
            DrawRectangle((int)b.x, (int)y,
                          (int)b.width, 2,
                          (Color){ 180, 120, 60, 255 });
        }
    }
}