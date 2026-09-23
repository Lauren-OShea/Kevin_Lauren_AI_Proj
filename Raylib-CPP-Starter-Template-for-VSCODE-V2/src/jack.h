#ifndef JACK_H
#define JACK_H

#include "raylib.h"

typedef struct Jack {
    Vector2 position;
    float baseY;
    float bobPhase;
    float bobOffset;
} Jack;

// Initialize Jack to default state
void Jack_Init(Jack *jack);

// Update Jack's position (mouse follow + bob)
void Jack_Update(Jack *jack);

// Draw Jack at his current position
void Jack_Draw(const Jack *jack);

#endif // JACK_H