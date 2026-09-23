#include "jack.h"
#include <math.h>

void Jack_Init(Jack *jack) {
    jack->position = (Vector2){ 130, GetScreenHeight() / 2.0f };
    jack->baseY = GetScreenHeight() / 2.0f;
    jack->bobPhase = 0.0f;
    jack->bobOffset = 0.0f;
}

void Jack_Update(Jack *jack) {
    Vector2 mousePos = GetMousePosition();
    float targetY = mousePos.y;

    if (targetY < 40) targetY = 40;
    if (targetY > GetScreenHeight() - 40) targetY = GetScreenHeight() - 40;

    // Smooth follow
    jack->position.y += (targetY - jack->position.y) * 0.12f;

    // Idle bob
    jack->bobPhase += 0.08f;
    jack->bobOffset = sinf(jack->bobPhase) * 7.0f;
}

void Jack_Draw(const Jack *jack) {
    float drawY = jack->position.y + jack->bobOffset;

    // Shadow / Glow
    DrawCircle(jack->position.x, drawY, 22, (Color){ 176, 224, 255, 80 });
    DrawCircle(jack->position.x, drawY, 18, (Color){ 229, 244, 255, 200 });

    // Body
    DrawEllipse(jack->position.x, drawY, 22, 28, (Color){ 229, 244, 255, 255 });

    // Core
    DrawEllipse(jack->position.x - 2, drawY - 4, 12, 16, WHITE);

    // Eyes
    DrawCircle(jack->position.x - 8, drawY - 8, 4, (Color){ 27, 59, 78, 255 });
    DrawCircle(jack->position.x + 8, drawY - 8, 4, (Color){ 27, 59, 78, 255 });

    // Pupils
    DrawCircle(jack->position.x - 9, drawY - 10, 1.6f, (Color){ 212, 240, 255, 255 });
    DrawCircle(jack->position.x + 7, drawY - 10, 1.6f, (Color){ 212, 240, 255, 255 });

    // Smile
    DrawCircleLines(jack->position.x, drawY + 2, 9, (Color){ 27, 59, 78, 255 });

    // Ice Crystals on head
    DrawTriangle(
        (Vector2){ jack->position.x - 12, drawY - 28 },
        (Vector2){ jack->position.x - 6,  drawY - 40 },
        (Vector2){ jack->position.x,      drawY - 28 },
        (Color){ 179, 228, 255, 255 }
    );
    DrawTriangle(
        (Vector2){ jack->position.x + 12, drawY - 28 },
        (Vector2){ jack->position.x + 6,  drawY - 40 },
        (Vector2){ jack->position.x,      drawY - 28 },
        (Color){ 179, 228, 255, 255 }
    );
}