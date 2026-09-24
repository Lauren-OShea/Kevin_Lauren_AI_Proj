#ifndef JACK_H
#define JACK_H

#include "raylib.h"
#include "platform.h"

typedef struct Jack {
    Vector2 position;
    float radiusX;
    float radiusY;
    float baseY;
    float bobPhase;
    float bobOffset;


    float velocityY;
    bool isJumping;
    float moveSpeed;
    float jumpForce;
    float gravity;

    bool onGround;

} Jack;

// Initialize Jack to default state
void Jack_Init(Jack *jack);

// Update Jack's position (mouse follow + bob)
void Jack_Update(Jack *jack, const Platform platforms[MAX_PLATFORMS]);

// Draw Jack at his current position
void Jack_Draw(const Jack *jack);

#endif // JACK_H