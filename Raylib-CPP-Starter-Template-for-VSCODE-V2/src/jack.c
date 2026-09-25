#include "jack.h"
#include <math.h>

/* ============================================================
 *  Animation data
 *
 *  Frames per strip:
 *    idle        : 6
 *    walk        : 8
 *    jump        : 5
 *    shoot       : 5
 *    knockedDown : 8
 *    down        : 1
 *    getUp       : 5
 * ============================================================ */

static const int JACK_ANIM_FRAMES[JACK_ANIM_COUNT] = {
    6,   /* IDLE         */
    8,   /* WALK         */
    5,   /* JUMP         */
    5,   /* SHOOT        */
    8,   /* KNOCKED_DOWN */
    1,   /* DOWN         */
    5,   /* GET_UP       */
};

static const float JACK_ANIM_FRAME_DT[JACK_ANIM_COUNT] = {
    0.15f,   /* IDLE         */
    0.10f,   /* WALK         */
    0.12f,   /* JUMP         */
    0.08f,   /* SHOOT        */
    0.09f,   /* KNOCKED_DOWN */
    0.30f,   /* DOWN         */
    0.12f,   /* GET_UP       */
};

/* Per-animation Y offset (positive = lower / closer to ground).
 * Tune any entry if a specific animation sits too high or low. */
static const float JACK_ANIM_Y_OFFSET[JACK_ANIM_COUNT] = {
     0.0f,   /* IDLE         */
     0.0f,   /* WALK         */
     0.0f,   /* JUMP         */
     0.0f,   /* SHOOT        */
     0.0f,   /* KNOCKED_DOWN */
     0.0f,   /* DOWN         */
     0.0f,   /* GET_UP       */
};

static const char *PLAYER_SPRITE_PATHS[JACK_ANIM_COUNT] = {
    "assets/player/idle.png",
    "assets/player/walk.png",
    "assets/player/jump.png",
    "assets/player/shoot.png",
    "assets/player/knockedDown.png",
    "assets/player/down.png",
    "assets/player/getUp.png",
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
            TraceLog(LOG_INFO, "Loaded %s (%dx%d)",
                     PLAYER_SPRITE_PATHS[i], tex.width, tex.height);
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

    jack->velocityY = 0.0f;
    jack->moveSpeed = 4.0f;
    jack->jumpForce = 12.0f;
    jack->gravity   = 0.7f;
    jack->facingDir = 1.0f;
    jack->onGround  = false;

    jack->state       = JACK_STATE_NORMAL;
    jack->currentAnim = JACK_ANIM_IDLE;
    jack->animTimer   = 0.0f;
    jack->stateTimer  = 0.0f;
    jack->invulnTimer = 0.0f;

    jack->tintColor   = tintColor;
    jack->feetOffsetY = JACK_FEET_OFFSET;
}

/* ============================================================
 *  State / animation setters
 *  — each does exactly ONE thing. Timers reset only on change.
 * ============================================================ */

static void SetState(Jack *jack, JackState s) {
    if (jack->state == s) return;
    jack->state      = s;
    jack->stateTimer = 0.0f;
}

static void SetAnim(Jack *jack, JackAnim a) {
    if (jack->currentAnim == a) return;
    jack->currentAnim = a;
    jack->animTimer   = 0.0f;
}

/* ============================================================
 *  Public state triggers
 * ============================================================ */

void Jack_TriggerShoot(Jack *jack) {
    if (jack->state != JACK_STATE_NORMAL) return;
    SetState(jack, JACK_STATE_SHOOTING);
    SetAnim (jack, JACK_ANIM_SHOOT);
}

void Jack_Knockdown(Jack *jack) {
    if (jack->state == JACK_STATE_KNOCKED_DOWN ||
        jack->state == JACK_STATE_DOWN         ||
        jack->state == JACK_STATE_GETTING_UP) return;

    SetState(jack, JACK_STATE_KNOCKED_DOWN);
    SetAnim (jack, JACK_ANIM_KNOCKED_DOWN);
}

bool Jack_IsVulnerable(const Jack *jack) {
    /* Not vulnerable during the knockdown sequence OR during
     * the post-getup grace window. */
    if (jack->state == JACK_STATE_KNOCKED_DOWN ||
        jack->state == JACK_STATE_DOWN         ||
        jack->state == JACK_STATE_GETTING_UP) return false;
    if (jack->invulnTimer > 0.0f)             return false;
    return true;
}

bool Jack_IsInvulnerable(const Jack *jack) {
    if (jack->state == JACK_STATE_KNOCKED_DOWN ||
        jack->state == JACK_STATE_DOWN         ||
        jack->state == JACK_STATE_GETTING_UP) return true;
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

    bool canControl = (jack->state == JACK_STATE_NORMAL ||
                       jack->state == JACK_STATE_SHOOTING);
    bool inKnockdown = (jack->state == JACK_STATE_KNOCKED_DOWN ||
                        jack->state == JACK_STATE_DOWN         ||
                        jack->state == JACK_STATE_GETTING_UP);
    bool moving = false;

    /* ---------- Horizontal input ---------- */
    if (canControl) {
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
    }

    /* ---------- Horizontal collision vs platform sides ---------- */
    if (!inKnockdown && jack->velocityY >= 0.0f) {
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
    if (canControl && IsKeyPressed(controls->keyJump) && jack->onGround) {
        jack->velocityY = -jack->jumpForce;
        jack->onGround  = false;
    }

    /* ---------- Gravity ---------- */
    jack->velocityY += jack->gravity;

    float beforeBottomMove = Jack_Bottom(jack);
    jack->position.y += jack->velocityY;
    bool landed = false;

    /* ---------- One-way platform landing ---------- */
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

    /* ---------- Screen floor & ceiling ---------- */
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

    /* ============================================================
     *  Timers
     * ============================================================ */

    jack->animTimer  += dt;
    jack->stateTimer += dt;

    if (jack->invulnTimer > 0.0f) {
        jack->invulnTimer -= dt;
        if (jack->invulnTimer < 0.0f) jack->invulnTimer = 0.0f;
    }

    /* ============================================================
     *  State transitions
     * ============================================================ */

    switch (jack->state) {
        case JACK_STATE_SHOOTING:
            if (jack->stateTimer >= AnimDuration(JACK_ANIM_SHOOT))
                SetState(jack, JACK_STATE_NORMAL);
            break;

        case JACK_STATE_KNOCKED_DOWN:
            if (jack->stateTimer >= AnimDuration(JACK_ANIM_KNOCKED_DOWN))
                SetState(jack, JACK_STATE_DOWN);
            break;

        case JACK_STATE_DOWN:
            if (jack->stateTimer >= JACK_DOWN_DURATION)
                SetState(jack, JACK_STATE_GETTING_UP);
            break;

        case JACK_STATE_GETTING_UP:
            if (jack->stateTimer >= AnimDuration(JACK_ANIM_GET_UP)) {
                jack->invulnTimer = JACK_POST_GETUP_INVULN;   /* 2s blink */
                SetState(jack, JACK_STATE_NORMAL);
            }
            break;

        default: break;
    }

    /* ============================================================
     *  Animation selection
     * ============================================================ */

    JackAnim target;
    switch (jack->state) {
        case JACK_STATE_KNOCKED_DOWN: target = JACK_ANIM_KNOCKED_DOWN; break;
        case JACK_STATE_DOWN:         target = JACK_ANIM_DOWN;         break;
        case JACK_STATE_GETTING_UP:   target = JACK_ANIM_GET_UP;       break;
        case JACK_STATE_SHOOTING:     target = JACK_ANIM_SHOOT;        break;
        default:
            if (!jack->onGround) target = JACK_ANIM_JUMP;
            else if (moving)     target = JACK_ANIM_WALK;
            else                 target = JACK_ANIM_IDLE;
            break;
    }
    SetAnim(jack, target);
}

/* ============================================================
 *  Draw
 * ============================================================ */

void Jack_Draw(const Jack *jack, const PlayerSprites *sprites) {
    JackAnim anim = jack->currentAnim;
    if (!sprites->loaded[anim]) return;

    int   frameCount = JACK_ANIM_FRAMES[anim];
    int   frameW     = sprites->frameW[anim];
    int   frameH     = sprites->frameH[anim];
    float frameDt    = JACK_ANIM_FRAME_DT[anim];

    /* Modulo so idle / walk loop forever.
     * One-shot animations still work because their state transitions
     * fire before the strip overruns. `down` is 1 frame → always 0. */
    int frame = 0;
    if (frameCount > 1) {
        int raw = (int)(jack->animTimer / frameDt);
        if (raw < 0) raw = 0;
        frame = raw % frameCount;
    }

    /* Source rect (flip horizontally if facing left) */
    float srcX = (float)(frame * frameW);
    Rectangle src;
    if (jack->facingDir < 0.0f) {
        src = (Rectangle){ srcX + frameW, 0.0f,
                           -(float)frameW, (float)frameH };
    } else {
        src = (Rectangle){ srcX, 0.0f,
                           (float)frameW, (float)frameH };
    }

    /* Destination — bottom-center aligned to feet */
    float drawW = frameW * SPRITE_SCALE;
    float drawH = frameH * SPRITE_SCALE;
    float drawX = jack->position.x - drawW * 0.5f;
    float drawY = jack->position.y + jack->radiusY
                  - drawH + jack->feetOffsetY
                  + JACK_ANIM_Y_OFFSET[anim];

    Rectangle dst = { drawX, drawY, drawW, drawH };

    /* ---------- Tint + invulnerability blink ---------- */
    Color tint = jack->tintColor;

    if (Jack_IsInvulnerable(jack)) {
        float blink = 0.5f + 0.5f * sinf((float)GetTime() * 22.0f);
        tint.a = (unsigned char)(120 + 135 * blink);   /* 120..255 */
    }

    DrawTexturePro(sprites->tex[anim], src, dst, (Vector2){ 0, 0 }, 0.0f, tint);
}