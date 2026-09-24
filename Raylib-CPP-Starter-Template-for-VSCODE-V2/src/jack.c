#include "jack.h"
#include <math.h>

void Jack_Init(Jack *jack) {
    jack->position = (Vector2){ 130, 400 };
    jack->radiusX = 22.0f;
    jack->radiusY = 28.0f;
    jack->bobPhase = 0.0f;
    jack->bobOffset = 0.0f;

    jack->velocityY = 0.0f;
    jack->moveSpeed = 4.0f; // pixels per second
    jack->jumpForce = 12.0f; // initial jump velocity
    jack->gravity = 0.7f;   // pixels per second squared
    jack->onGround = false;


}

static inline float Jack_Bottom(const Jack *j) {
    return j->position.y + j->radiusY;
}

void Jack_Update(Jack *jack, const Platform platforms[MAX_PLATFORMS]) {
   if(IsKeyDown(KEY_A)) jack->position.x -=jack->moveSpeed;
   if(IsKeyDown(KEY_D)) jack->position.x +=jack->moveSpeed;

   if(jack->position.x < jack->radiusX) 
   jack->position.x = jack->radiusX;
    if(jack->position.x > GetScreenWidth() - jack->radiusX)
    jack->position.x = GetScreenWidth() - jack->radiusX;


if(IsKeyPressed(KEY_SPACE) && jack->onGround)
{
    jack->velocityY = -jack->jumpForce;
    jack->isJumping = true;
    jack->onGround = false;

}

jack->velocityY += jack->gravity;

float beforeBottomMove = Jack_Bottom(jack);
jack->position.y += jack->velocityY;    
bool landed = false;


if (jack->velocityY >= 0.0f) {
        // Build Jack's AABB
        Rectangle jackBox = {
            jack->position.x - jack->radiusX,
            jack->position.y - jack->radiusY,
            jack->radiusX * 2,
            jack->radiusY * 2
        };

        for (int i = 0; i < MAX_PLATFORMS; i++) {
            if (!platforms[i].active) continue;

            Rectangle p = platforms[i].bounds;

            // Horizontal overlap?
            if (jackBox.x + jackBox.width <= p.x) continue;
            if (jackBox.x >= p.x + p.width)       continue;

            // Only land if we were ABOVE the platform top last frame
            // and are now at/below it. This is what makes it one-way.

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

    if (!landed) {
        jack->onGround = false;
    }

    // ----- Floor of the screen (fallback ground) -----
    float screenFloor = GetScreenHeight() - jack->radiusY;
    if (jack->position.y >= screenFloor) {
        jack->position.y = screenFloor;
        jack->velocityY  = 0.0f;
        jack->onGround   = true;
    }

    // ----- Ceiling of the screen -----
    if (jack->position.y < jack->radiusY) {
        jack->position.y = jack->radiusY;
        if (jack->velocityY < 0) jack->velocityY = 0.0f;
    }

    // ----- Idle bob (only when grounded) -----
    if (jack->onGround) {
        jack->bobPhase += 0.08f;
        jack->bobOffset = sinf(jack->bobPhase) * 7.0f;
    } else {
        jack->bobOffset = 0.0f;
    }
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