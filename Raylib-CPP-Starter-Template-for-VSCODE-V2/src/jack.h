#ifndef JACK_H
#define JACK_H

#include "raylib.h"
#include "platform.h"

typedef struct JackControls {
    int keyLeft;
    int keyRight;
    int keyJump;
} JackControls;

typedef struct Jack {
    Vector2 position;
    float   radiusX;
    float   radiusY;
    float   baseY;
    float   bobPhase;
    float   bobOffset;

    float   velocityY;
    bool    isJumping;
    float   moveSpeed;
    float   jumpForce;
    float   gravity;
    float   facingDir;   /* +1 = right, -1 = left */

    bool    onGround;

    Color   accentColor; /* used for crystals / glow */
} Jack;

void Jack_Init  (Jack *jack, Vector2 startPos, Color accentColor);
void Jack_Update(Jack *jack, const Platform platforms[MAX_PLATFORMS],
                 const JackControls *controls);
void Jack_Draw  (const Jack *jack);

#endif /* JACK_H */