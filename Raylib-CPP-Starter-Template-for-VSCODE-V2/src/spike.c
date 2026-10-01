#include "spike.h"

void Spike_InitAll(Spike spikes[MAX_SPIKES]) {
    for (int i = 0; i < MAX_SPIKES; i++) spikes[i].active = false;
}

bool Spike_Add(Spike spikes[MAX_SPIKES], Rectangle bounds) {
    for (int i = 0; i < MAX_SPIKES; i++) {
        if (!spikes[i].active) {
            spikes[i].bounds = bounds;
            spikes[i].active = true;
            return true;
        }
    }
    return false;
}

bool Spike_OverlapsRect(const Spike spikes[MAX_SPIKES], Rectangle rect) {
    for (int i = 0; i < MAX_SPIKES; i++) {
        if (!spikes[i].active) continue;
        if (CheckCollisionRecs(spikes[i].bounds, rect)) return true;
    }
    return false;
}

void Spike_DrawAll(const Spike spikes[MAX_SPIKES]) {
    for (int i = 0; i < MAX_SPIKES; i++) {
        if (!spikes[i].active) continue;

        Rectangle b = spikes[i].bounds;

        DrawRectangleRec(b, (Color){ 40, 40, 50, 255 });

        int numSpikes = (int)(b.width / 8.0f);
        if (numSpikes < 1) numSpikes = 1;
        float spikeW = b.width / numSpikes;

        for (int k = 0; k < numSpikes; k++) {
            float sx = b.x + k * spikeW;

            DrawTriangle(
                (Vector2){ sx,                  b.y + b.height },
                (Vector2){ sx + spikeW,         b.y + b.height },
                (Vector2){ sx + spikeW * 0.5f,  b.y },
                (Color){ 180, 190, 210, 255 }
            );
            DrawTriangle(
                (Vector2){ sx + spikeW * 0.15f, b.y + b.height },
                (Vector2){ sx + spikeW * 0.5f,  b.y },
                (Vector2){ sx + spikeW * 0.5f,  b.y + b.height },
                (Color){ 230, 240, 255, 255 }
            );
        }

        DrawRectangle((int)b.x, (int)(b.y + b.height - 3),
                      (int)b.width, 3, (Color){ 20, 20, 30, 255 });
    }
}