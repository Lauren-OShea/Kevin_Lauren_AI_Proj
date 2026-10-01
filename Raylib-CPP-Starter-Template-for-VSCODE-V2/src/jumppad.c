#include "jumppad.h"
#include <math.h>

void JumpPad_InitAll(JumpPad pads[MAX_JUMPPADS]) {
    for (int i = 0; i < MAX_JUMPPADS; i++) pads[i].active = false;
}

bool JumpPad_Add(JumpPad pads[MAX_JUMPPADS], Rectangle bounds) {
    for (int i = 0; i < MAX_JUMPPADS; i++) {
        if (!pads[i].active) {
            pads[i].bounds    = bounds;
            pads[i].animTimer = 0.0f;
            pads[i].active    = true;
            return true;
        }
    }
    return false;
}

void JumpPad_UpdateAll(JumpPad pads[MAX_JUMPPADS], float dt) {
    for (int i = 0; i < MAX_JUMPPADS; i++) {
        if (!pads[i].active) continue;
        pads[i].animTimer += dt;
    }
}

bool JumpPad_TryTrigger(const JumpPad pads[MAX_JUMPPADS], Rectangle entityRect) {
    for (int i = 0; i < MAX_JUMPPADS; i++) {
        if (!pads[i].active) continue;

        Rectangle detect = {
            pads[i].bounds.x,
            pads[i].bounds.y - JUMPPAD_DETECT_PAD,
            pads[i].bounds.width,
            pads[i].bounds.height + JUMPPAD_DETECT_PAD
        };

        if (CheckCollisionRecs(detect, entityRect)) return true;
    }
    return false;
}

void JumpPad_DrawAll(const JumpPad pads[MAX_JUMPPADS]) {
    for (int i = 0; i < MAX_JUMPPADS; i++) {
        if (!pads[i].active) continue;

        Rectangle b     = pads[i].bounds;
        float     pulse = 0.6f + 0.4f * sinf(pads[i].animTimer * 6.0f);

        DrawRectangleRec(b, (Color){ 25, 35, 55, 255 });

        unsigned char g = (unsigned char)(180 + 60 * pulse);
        DrawRectangle((int)b.x, (int)b.y, (int)b.width, 4,
                      (Color){ 120, g, 255, 255 });

        Color arrowCol = (Color){ 255, 255, 255, (unsigned char)(180 * pulse) };
        for (int k = 0; k < 3; k++) {
            float cx = b.x + 6 + k * 12;
            DrawTriangle(
                (Vector2){ cx,     b.y + 8 },
                (Vector2){ cx + 8, b.y + 8 },
                (Vector2){ cx + 4, b.y + 2 },
                arrowCol
            );
        }

        DrawRectangle((int)b.x, (int)b.y, 2, (int)b.height,
                      (Color){ 200, 220, 255, 255 });
        DrawRectangle((int)(b.x + b.width - 2), (int)b.y, 2, (int)b.height,
                      (Color){ 200, 220, 255, 255 });
    }
}