#include "jack.h"
#include <math.h>

void Jack_Init(Jack *jack, Vector2 startPos, Color accentColor) {
    jack->position  = startPos;
    jack->radiusX   = 22.0f;
    jack->radiusY   = 28.0f;
    jack->bobPhase  = 0.0f;
    jack->bobOffset = 0.0f;

    jack->velocityY = 0.0f;
    jack->moveSpeed = 4.0f;
    jack->jumpForce = 12.0f;
    jack->gravity   = 0.7f;
    jack->facingDir = 1.0f;
    jack->onGround  = false;

    jack->accentColor = accentColor;
}

static inline float Jack_Bottom(const Jack *j) {
    return j->position.y + j->radiusY;
}

void Jack_Update(Jack *jack, const Platform platforms[MAX_PLATFORMS],
                 const JackControls *controls)
{
    /* ---------- Horizontal input + facing ---------- */
    if (IsKeyDown(controls->keyLeft)) {
        jack->position.x -= jack->moveSpeed;
        jack->facingDir = -1.0f;
    }
    if (IsKeyDown(controls->keyRight)) {
        jack->position.x += jack->moveSpeed;
        jack->facingDir = 1.0f;
    }

    /* ---------- Horizontal collision vs platform sides ----------
     * Skipped while moving upward so one-way platforms work.
     * Skipped when feet are at/near the top (means he is standing on it). */
    if (jack->velocityY >= 0.0f) {
        for (int i = 0; i < MAX_PLATFORMS; i++) {
            if (!platforms[i].active) continue;
            Rectangle p = platforms[i].bounds;

            Rectangle jBox = {
                jack->position.x - jack->radiusX,
                jack->position.y - jack->radiusY,
                jack->radiusX * 2,
                jack->radiusY * 2
            };
            if (!CheckCollisionRecs(jBox, p)) continue;
            if (Jack_Bottom(jack) <= p.y + 6.0f) continue;

            float platCX = p.x + p.width * 0.5f;
            if (jack->position.x < platCX) {
                jack->position.x = p.x - jack->radiusX;
            } else {
                jack->position.x = p.x + p.width + jack->radiusX;
            }
        }
    }

    /* ---------- Screen X clamp ---------- */
    if (jack->position.x < jack->radiusX)
        jack->position.x = jack->radiusX;
    if (jack->position.x > GetScreenWidth() - jack->radiusX)
        jack->position.x = GetScreenWidth() - jack->radiusX;

    /* ---------- Jump ---------- */
    if (IsKeyPressed(controls->keyJump) && jack->onGround) {
        jack->velocityY = -jack->jumpForce;
        jack->isJumping = true;
        jack->onGround  = false;
    }

    /* ---------- Gravity ---------- */
    jack->velocityY += jack->gravity;

    float beforeBottomMove = Jack_Bottom(jack);
    jack->position.y += jack->velocityY;
    bool landed = false;

    /* ---------- One-way platform landing (only while falling) ---------- */
    if (jack->velocityY >= 0.0f) {
        Rectangle jackBox = {
            jack->position.x - jack->radiusX,
            jack->position.y - jack->radiusY,
            jack->radiusX * 2,
            jack->radiusY * 2
        };

        for (int i = 0; i < MAX_PLATFORMS; i++) {
            if (!platforms[i].active) continue;
            Rectangle p = platforms[i].bounds;

            if (jackBox.x + jackBox.width <= p.x) continue;
            if (jackBox.x >= p.x + p.width)       continue;

            if (beforeBottomMove <= p.y + 1.0f && Jack_Bottom(jack) >= p.y) {
                jack->position.y = p.y - jack->radiusY;
                jack->velocityY  = 0.0f;
                jack->isJumping  = false;
                jack->onGround   = true;
                landed = true;
                break;
            }
        }
    }

    if (!landed) jack->onGround = false;

    /* ---------- Screen floor ---------- */
    float screenFloor = GetScreenHeight() - jack->radiusY;
    if (jack->position.y >= screenFloor) {
        jack->position.y = screenFloor;
        jack->velocityY  = 0.0f;
        jack->onGround   = true;
    }

    /* ---------- Screen ceiling ---------- */
    if (jack->position.y < jack->radiusY) {
        jack->position.y = jack->radiusY;
        if (jack->velocityY < 0) jack->velocityY = 0.0f;
    }

    /* ---------- Idle bob ---------- */
    if (jack->onGround) {
        jack->bobPhase += 0.08f;
        jack->bobOffset = sinf(jack->bobPhase) * 7.0f;
    } else {
        jack->bobOffset = 0.0f;
    }
}

void Jack_Draw(const Jack *jack) {
    float drawY = jack->position.y + jack->bobOffset;
    Color acc   = jack->accentColor;

    /* Glow */
    DrawCircle(jack->position.x, drawY, 22, Fade(acc, 0.30f));
    DrawCircle(jack->position.x, drawY, 18, (Color){ 229, 244, 255, 200 });

    /* Body */
    DrawEllipse(jack->position.x, drawY, 22, 28, (Color){ 229, 244, 255, 255 });

    /* Core */
    DrawEllipse(jack->position.x - 2, drawY - 4, 12, 16, WHITE);

    /* Eyes */
    DrawCircle(jack->position.x - 8, drawY - 8, 4, (Color){ 27, 59, 78, 255 });
    DrawCircle(jack->position.x + 8, drawY - 8, 4, (Color){ 27, 59, 78, 255 });

    /* Pupils */
    DrawCircle(jack->position.x - 9, drawY - 10, 1.6f, (Color){ 212, 240, 255, 255 });
    DrawCircle(jack->position.x + 7, drawY - 10, 1.6f, (Color){ 212, 240, 255, 255 });

    /* Smile */
    DrawCircleLines(jack->position.x, drawY + 2, 9, (Color){ 27, 59, 78, 255 });

    /* Ice crystals — tinted to identify the player */
    DrawTriangle(
        (Vector2){ jack->position.x - 12, drawY - 28 },
        (Vector2){ jack->position.x - 6,  drawY - 40 },
        (Vector2){ jack->position.x,      drawY - 28 },
        acc
    );
    DrawTriangle(
        (Vector2){ jack->position.x + 12, drawY - 28 },
        (Vector2){ jack->position.x + 6,  drawY - 40 },
        (Vector2){ jack->position.x,      drawY - 28 },
        acc
    );

    /* Facing indicator — small crystal in front of Jack */
    float fx = jack->position.x + jack->facingDir * 26.0f;
    DrawCircle((int)fx, (int)(drawY - 2), 3, Fade(acc, 0.9f));
}