#include "movingplatform.h"
#include <math.h>

void MovingPlatform_InitAll(MovingPlatform platforms[MAX_MOVING_PLATFORMS]) {
    for (int i = 0; i < MAX_MOVING_PLATFORMS; i++) platforms[i].active = false;
}

bool MovingPlatform_Add(MovingPlatform platforms[MAX_MOVING_PLATFORMS],
                        Rectangle startBounds, Rectangle endBounds,
                        float speed)
{
    for (int i = 0; i < MAX_MOVING_PLATFORMS; i++) {
        if (!platforms[i].active) {
            platforms[i].bounds   = startBounds;
            platforms[i].startPos = (Vector2){ startBounds.x, startBounds.y };
            platforms[i].endPos   = (Vector2){ endBounds.x,   endBounds.y   };
            platforms[i].prevPos  = platforms[i].startPos;
            platforms[i].t        = 0.0f;
            platforms[i].dir      = 1.0f;
            platforms[i].speed    = speed;
            platforms[i].active   = true;
            return true;
        }
    }
    return false;
}

void MovingPlatform_UpdateAll(MovingPlatform platforms[MAX_MOVING_PLATFORMS],
                              float dt)
{
    for (int i = 0; i < MAX_MOVING_PLATFORMS; i++) {
        if (!platforms[i].active) continue;

        MovingPlatform *mp = &platforms[i];

        mp->prevPos = (Vector2){ mp->bounds.x, mp->bounds.y };

        mp->t += mp->dir * mp->speed * dt;
        if (mp->t >= 1.0f)      { mp->t = 1.0f; mp->dir = -1.0f; }
        else if (mp->t <= 0.0f) { mp->t = 0.0f; mp->dir =  1.0f; }

        mp->bounds.x = mp->startPos.x + (mp->endPos.x - mp->startPos.x) * mp->t;
        mp->bounds.y = mp->startPos.y + (mp->endPos.y - mp->startPos.y) * mp->t;
    }
}

Rectangle MovingPlatform_GetSweptBox(const MovingPlatform *mp) {
    float minX = fminf(mp->startPos.x, mp->endPos.x);
    float minY = fminf(mp->startPos.y, mp->endPos.y);
    float maxX = fmaxf(mp->startPos.x, mp->endPos.x) + mp->bounds.width;
    float maxY = fmaxf(mp->startPos.y, mp->endPos.y) + mp->bounds.height;
    return (Rectangle){ minX, minY, maxX - minX, maxY - minY };
}

void MovingPlatform_DrawAll(const MovingPlatform platforms[MAX_MOVING_PLATFORMS]) {
    for (int i = 0; i < MAX_MOVING_PLATFORMS; i++) {
        if (!platforms[i].active) continue;
        Rectangle b = platforms[i].bounds;

        DrawRectangleRec(b, (Color){ 60, 90, 130, 255 });

        DrawRectangle((int)b.x, (int)b.y, (int)b.width, 5,
                      (Color){ 160, 200, 255, 255 });

        DrawRectangle((int)b.x, (int)(b.y + b.height - 5), (int)b.width, 5,
                      (Color){ 30, 50, 80, 255 });

        float dx = platforms[i].endPos.x - platforms[i].startPos.x;
        float dy = platforms[i].endPos.y - platforms[i].startPos.y;

        int numArrows = 3;
        for (int k = 0; k < numArrows; k++) {
            float cx = b.x + 10 + k * (b.width - 20) / (numArrows - 1);
            float cy = b.y + b.height * 0.5f;

            if (fabsf(dx) >= fabsf(dy)) {
                float s = (dx > 0) ? 1.0f : -1.0f;
                DrawTriangle(
                    (Vector2){ cx + s * 4, cy },
                    (Vector2){ cx - s * 3, cy - 4 },
                    (Vector2){ cx - s * 3, cy + 4 },
                    (Color){ 200, 220, 255, 200 }
                );
            } else {
                float s = (dy > 0) ? 1.0f : -1.0f;
                DrawTriangle(
                    (Vector2){ cx,     cy + s * 4 },
                    (Vector2){ cx - 4, cy - s * 3 },
                    (Vector2){ cx + 4, cy - s * 3 },
                    (Color){ 200, 220, 255, 200 }
                );
            }
        }

        DrawRectangleLinesEx(b, 1.5f, (Color){ 180, 220, 255, 255 });
    }
}