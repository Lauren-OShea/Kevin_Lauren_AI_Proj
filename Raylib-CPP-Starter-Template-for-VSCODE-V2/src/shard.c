#include "shard.h"

void Shard_InitAll(Shard shards[MAX_SHARDS]) {
    for (int i = 0; i < MAX_SHARDS; i++) {
        shards[i].active = false;
    }
}

void Shard_Shoot(Shard shards[MAX_SHARDS], Vector2 origin) {
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (!shards[i].active) {
            shards[i].position = (Vector2){ origin.x + 15, origin.y };
            shards[i].velocity = (Vector2){ SHARD_SPEED, 0 };
            shards[i].active = true;
            return;
        }
    }
}

void Shard_UpdateAll(Shard shards[MAX_SHARDS], int screenWidth) {
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (!shards[i].active) continue;

        shards[i].position.x += shards[i].velocity.x;

        if (shards[i].position.x > screenWidth + 50) {
            shards[i].active = false;
        }
    }
}

void Shard_DrawAll(const Shard shards[MAX_SHARDS]) {
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (!shards[i].active) continue;

        Vector2 p = shards[i].position;
        float r = SHARD_RADIUS;

        // Glow
        DrawCircle(p.x, p.y, r + 4, (Color){ 170, 224, 255, 100 });

        // Shard shape (Diamond)
        DrawTriangle(
            (Vector2){ p.x + r * 0.9f, p.y },
            (Vector2){ p.x,            p.y - r * 0.8f },
            (Vector2){ p.x - r * 0.7f, p.y },
            (Color){ 214, 240, 255, 255 }
        );
        DrawTriangle(
            (Vector2){ p.x + r * 0.9f, p.y },
            (Vector2){ p.x - r * 0.7f, p.y },
            (Vector2){ p.x,            p.y + r * 0.8f },
            (Color){ 214, 240, 255, 255 }
        );

        // Highlight
        DrawTriangle(
            (Vector2){ p.x + r * 0.5f, p.y - 2 },
            (Vector2){ p.x,            p.y - r * 0.5f },
            (Vector2){ p.x - r * 0.4f, p.y - 2 },
            WHITE
        );
    }
}