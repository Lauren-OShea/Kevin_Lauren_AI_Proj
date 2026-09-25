#include "jack.h"
#include <math.h>

/* ============================================================
 *  Animation metadata
 * ============================================================ */

static const int JACK_ANIM_FRAMES[JACK_ANIM_COUNT] = {
    6,   /* IDLE  */
    8,   /* WALK  */
    5,   /* JUMP  */
    5,   /* SHOOT */
};

static const float JACK_ANIM_FRAME_DT[JACK_ANIM_COUNT] = {
    0.15f,   /* IDLE  */
    0.10f,   /* WALK  */
    0.12f,   /* JUMP  */
    0.08f,   /* SHOOT */
};

static const char *PLAYER_SPRITE_PATHS[JACK_ANIM_COUNT] = {
    "assets/player/idle.png",
    "assets/player/walk.png",
    "assets/player/jump.png",
    "assets/player/shoot.png",
};

static inline float AnimDuration(JackAnim a) {
    return JACK_ANIM_FRAMES[a] * JACK_ANIM_FRAME_DT[a];
}

/* ============================================================
 *  Sprite loading
 * ============================================================ */

void PlayerSprites_Load(PlayerSprites *sprites) {
    for (int i = 0; i < JACK_ANIM_COUNT; i++) {
        Texture2D tex = LoadTexture(PLAYER_SPRITE_PATHS[i]);

        if (tex.id == 0) {
            TraceLog(LOG_ERROR, "FAILED to load %s", PLAYER_SPRITE_PATHS[i]);
            Image img = GenImageColor(50, 50, MAGENTA);
            tex = LoadTextureFromImage(img);
            UnloadImage(img);
            sprites->loaded[i] = false;
        } else {
            SetTextureFilter(tex, TEXTURE_FILTER_POINT);
            sprites->loaded[i] = true;
            TraceLog(LOG_INFO, "Loaded %s (%dx%d, %d frames)",
                     PLAYER_SPRITE_PATHS[i],
                     tex.width, tex.height, JACK_ANIM_FRAMES[i]);
        }

        sprites->tex[i]    = tex;
        sprites->frameW[i] = tex.width / JACK_ANIM_FRAMES[i];
        sprites->frameH[i] = tex.height;
    }
}

void PlayerSprites_Unload(PlayerSprites *sprites) {
    for (int i = 0; i < JACK_ANIM_COUNT; i++) {
        if (sprites->tex[i].id != 0) {
            UnloadTexture(sprites->tex[i]);
            sprites->tex[i].id = 0;
        }
    }
}

/* ============================================================
 *  Init
 * ============================================================ */

void Jack_Init(Jack *jack, Vector2 startPos, Color tintColor) {
    jack->position  = startPos;
    jack->radiusX   = 22.0f;
    jack->radiusY   = 28.0f;

    jack->velocityX = 0.0f;
    jack->velocityY = 0.0f;
    jack->moveSpeed = 4.0f;
    jack->jumpForce = 12.0f;
    jack->gravity   = 0.7f;
    jack->facingDir = 1.0f;
    jack->onGround  = false;

    jack->state       = JACK_STATE_ALIVE;
    jack->currentAnim = JACK_ANIM_IDLE;
    jack->animTimer   = 0.0f;
    jack->shootTimer  = 0.0f;
    jack->invulnTimer = 0.0f;
    jack->lives       = JACK_MAX_LIVES;

    jack->tintColor   = tintColor;
    jack->feetOffsetY = JACK_FEET_OFFSET;
}

/* ============================================================
 *  Public triggers
 * ============================================================ */

void Jack_TriggerShoot(Jack *jack) {
    if (jack->state != JACK_STATE_ALIVE) return;
    if (jack->shootTimer > 0.0f) return;

    jack->shootTimer  = AnimDuration(JACK_ANIM_SHOOT);
    jack->currentAnim = JACK_ANIM_SHOOT;
    jack->animTimer   = 0.0f;
}

void Jack_Hit(Jack *jack) {
    if (!Jack_IsVulnerable(jack)) return;

    jack->lives--;
    jack->invulnTimer = JACK_INVULN_TIME;

    if (jack->lives <= 0) {
        /* Last life — fling them off the screen */
        jack->state     = JACK_STATE_FLUNG;
        jack->velocityX = -jack->facingDir * 9.0f;
        jack->velocityY = -11.0f;
        jack->onGround  = false;
    }
}

bool Jack_IsPlayable(const Jack *jack) {
    return jack->state == JACK_STATE_ALIVE;
}

bool Jack_IsVulnerable(const Jack *jack) {
    if (jack->state != JACK_STATE_ALIVE) return false;
    return jack->invulnTimer <= 0.0f;
}

bool Jack_IsInvulnerable(const Jack *jack) {
    if (jack->state != JACK_STATE_ALIVE) return false;
    return jack->invulnTimer > 0.0f;
}

/* ============================================================
 *  Update
 * ============================================================ */

static inline float Jack_Bottom(const Jack *j) {
    return j->position.y + j->radiusY;
}

void Jack_Update(Jack *jack, const Platform platforms[MAX_PLATFORMS],
                 const JackControls *controls)
{
    float dt = GetFrameTime();

    /* ============================================================
     *  Flung off-screen — fly, then disappear
     * ============================================================ */
    if (jack->state == JACK_STATE_FLUNG) {
        jack->velocityY += jack->gravity;
        jack->position.x += jack->velocityX;
        jack->position.y += jack->velocityY;
        jack->animTimer += dt;

        if (jack->position.x < -150.0f ||
            jack->position.x > GetScreenWidth() + 150.0f ||
            jack->position.y < -300.0f)
        {
            jack->state = JACK_STATE_GONE;
        }
        return;
    }

    if (jack->state == JACK_STATE_GONE) return;

    /* ============================================================
     *  Normal alive behaviour
     * ============================================================ */

    bool moving = false;

    if (IsKeyDown(controls->keyLeft)) {
        jack->position.x -= jack->moveSpeed;
        jack->facingDir = -1.0f;
        moving = true;
    }
    if (IsKeyDown(controls->keyRight)) {
        jack->position.x += jack->moveSpeed;
        jack->facingDir = 1.0f;
        moving = true;
    }

    /* Horizontal platform collision */
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

    /* Screen X clamp */
    if (jack->position.x < jack->radiusX)
        jack->position.x = jack->radiusX;
    if (jack->position.x > GetScreenWidth() - jack->radiusX)
        jack->position.x = GetScreenWidth() - jack->radiusX;

    /* Jump */
    if (IsKeyPressed(controls->keyJump) && jack->onGround) {
        jack->velocityY = -jack->jumpForce;
        jack->onGround  = false;
    }

    /* Gravity */
    jack->velocityY += jack->gravity;

    float beforeBottomMove = Jack_Bottom(jack);
    jack->position.y += jack->velocityY;
    bool landed = false;

    /* One-way platform landing */
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
                jack->onGround   = true;
                landed = true;
                break;
            }
        }
    }
    if (!landed) jack->onGround = false;

    /* Screen floor & ceiling */
    float screenFloor = GetScreenHeight() - jack->radiusY;
    if (jack->position.y >= screenFloor) {
        jack->position.y = screenFloor;
        jack->velocityY  = 0.0f;
        jack->onGround   = true;
    }
    if (jack->position.y < jack->radiusY) {
        jack->position.y = jack->radiusY;
        if (jack->velocityY < 0) jack->velocityY = 0.0f;
    }

    /* Timers */
    jack->animTimer += dt;

    if (jack->invulnTimer > 0.0f) {
        jack->invulnTimer -= dt;
        if (jack->invulnTimer < 0.0f) jack->invulnTimer = 0.0f;
    }
    if (jack->shootTimer > 0.0f) {
        jack->shootTimer -= dt;
        if (jack->shootTimer < 0.0f) jack->shootTimer = 0.0f;
    }

    /* Animation selection */
    JackAnim target;
    if (jack->shootTimer > 0.0f)   target = JACK_ANIM_SHOOT;
    else if (!jack->onGround)      target = JACK_ANIM_JUMP;
    else if (moving)               target = JACK_ANIM_WALK;
    else                           target = JACK_ANIM_IDLE;

    if (jack->currentAnim != target) {
        jack->currentAnim = target;
        jack->animTimer   = 0.0f;
    }
}

/* ============================================================
 *  Draw
 * ============================================================ */

void Jack_Draw(const Jack *jack, const PlayerSprites *sprites) {
    if (jack->state == JACK_STATE_GONE) return;

    JackAnim anim = jack->currentAnim;
    if (!sprites->loaded[anim]) return;

    int   frameCount = JACK_ANIM_FRAMES[anim];
    int   frameW     = sprites->frameW[anim];
    int   frameH     = sprites->frameH[anim];
    float frameDt    = JACK_ANIM_FRAME_DT[anim];

    int frame = 0;
    if (frameCount > 1) {
        int raw = (int)(jack->animTimer / frameDt);
        if (raw < 0) raw = 0;

        if (anim == JACK_ANIM_IDLE || anim == JACK_ANIM_WALK) {
            frame = raw % frameCount;
        } else {
            if (raw >= frameCount) raw = frameCount - 1;
            frame = raw;
        }
    }

    float srcX = (float)(frame * frameW);
    Rectangle src;
    if (jack->facingDir < 0.0f) {
        src = (Rectangle){ srcX + frameW, 0.0f,
                           -(float)frameW, (float)frameH };
    } else {
        src = (Rectangle){ srcX, 0.0f,
                           (float)frameW, (float)frameH };
    }

    float drawW = frameW * SPRITE_SCALE;
    float drawH = frameH * SPRITE_SCALE;
    float drawX = jack->position.x - drawW * 0.5f;
    float drawY = jack->position.y + jack->radiusY - drawH + jack->feetOffsetY;

    Rectangle dst = { drawX, drawY, drawW, drawH };

    Color tint = jack->tintColor;

    if (jack->state == JACK_STATE_FLUNG) {
        tint.a = 200;
    } else if (Jack_IsInvulnerable(jack)) {
        float blink = 0.5f + 0.5f * sinf((float)GetTime() * 20.0f);
        tint.a = (unsigned char)(80 + 175 * blink);
    }

    DrawTexturePro(sprites->tex[anim], src, dst, (Vector2){ 0, 0 }, 0.0f, tint);
}