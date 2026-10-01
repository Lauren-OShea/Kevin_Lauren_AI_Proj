#ifndef JACKFROST_MOVINGPLATFORM_H
#define JACKFROST_MOVINGPLATFORM_H

#include <stdbool.h>
#include "raylib.h"

#define MAX_MOVING_PLATFORMS 8

typedef struct MovingPlatform {
    Rectangle bounds;
    Vector2   startPos;
    Vector2   endPos;
    Vector2   prevPos;
    float     t;
    float     dir;
    float     speed;
    bool      active;
} MovingPlatform;

void MovingPlatform_InitAll(MovingPlatform platforms[MAX_MOVING_PLATFORMS]);

bool MovingPlatform_Add(MovingPlatform platforms[MAX_MOVING_PLATFORMS],
                        Rectangle startBounds, Rectangle endBounds,
                        float speed);

void MovingPlatform_UpdateAll(MovingPlatform platforms[MAX_MOVING_PLATFORMS],
                              float dt);

Rectangle MovingPlatform_GetSweptBox(const MovingPlatform *mp);

void MovingPlatform_DrawAll(const MovingPlatform platforms[MAX_MOVING_PLATFORMS]);

#endif /* JACKFROST_MOVINGPLATFORM_H */