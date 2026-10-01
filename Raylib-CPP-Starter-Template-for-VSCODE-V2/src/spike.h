#ifndef JACKFROST_SPIKE_H
#define JACKFROST_SPIKE_H

#include <stdbool.h>
#include "raylib.h"

#define MAX_SPIKES        64
#define SPIKE_WIDTH       30
#define SPIKE_HEIGHT      14
#define SPIKE_SAFE_MARGIN 20

typedef struct Spike {
    Rectangle bounds;
    bool      active;
} Spike;

void Spike_InitAll(Spike spikes[MAX_SPIKES]);
bool Spike_Add    (Spike spikes[MAX_SPIKES], Rectangle bounds);
void Spike_DrawAll(const Spike spikes[MAX_SPIKES]);

bool Spike_OverlapsRect(const Spike spikes[MAX_SPIKES], Rectangle rect);

#endif /* JACKFROST_SPIKE_H */