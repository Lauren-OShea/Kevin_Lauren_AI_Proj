#include "jack.h"
#include <math.h>
#include <stddef.h>

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
 *  Fling helper — random direction, upward-biased
 * ============================================================ */

static void Jack_ApplyRandomFling(Jack *jack, float minPop, float maxPop)
{
    float side = (GetRandomValue(0, 1) == 0) ? -1.0f : 1.0f;
    float mag  = (float)GetRandomValue(8, 14);

    jack->velocityX = side * mag;
    jack->velocityY = -(float)GetRandomValue((int)minPop, (int)maxPop);
    jack->onGround  = false;
    jack->animTimer = 0.0f;
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
    jack->position      = startPos;
    jack->spawnPosition = startPos;

    jack->radiusX   = 22.0f;
    jack->radiusY   = 28.0f;

    jack->velocityX = 0.0f;
    jack->velocityY = 0.0f;
    jack->moveSpeed = 4.0f;
    jack->jumpForce = 12.0f;
    jack->gravity   = 0.7f;
    jack->facingDir = 1.0f;
    jack->onGround  = false;

    jack->onLadder    = false;
    jack->climbing    = false;
    jack->ladderIndex = -1;

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
        jack->state     = JACK_STATE_FLUNG;
        Jack_ApplyRandomFling(jack, 16.0f, 22.0f);
        jack->velocityX = -jack->facingDir * 9.0f;
        jack->velocityY = -11.0f;
        jack->onGround  = false;
        jack->onLadder  = false;
        jack->climbing  = false;
        jack->ladderIndex = -1;
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

static int FindOverlappingLadder(const Jack *jack,
                                 const Ladder ladders[MAX_LADDERS],
                                 float expandX, float expandY)
{
    Rectangle jBox = {
        jack->position.x - jack->radiusX - expandX,
        jack->position.y - jack->radiusY - expandY,
        (jack->radiusX + expandX) * 2.0f,
        (jack->radiusY + expandY) * 2.0f
    };

    for (int i = 0; i < MAX_LADDERS; i++) {
        if (!ladders[i].active) continue;
        if (CheckCollisionRecs(jBox, ladders[i].bounds)) return i;
    }
    return -1;
}

void Jack_Update(Jack *jack,
                 const Platform       platforms[MAX_PLATFORMS],
                 const Ladder         ladders[MAX_LADDERS],
                 const MovingPlatform movingPlatforms[MAX_MOVING_PLATFORMS],
                 const JackControls  *controls,
                 int worldW, int worldH)
{
    float dt = GetFrameTime();
    Vector2 prevPosition = jack->position;

    /* ============================================================
     *  Flung off-screen — fly, then disappear
     * ============================================================ */
    if (jack->state == JACK_STATE_FLUNG) {
        jack->velocityY += jack->gravity;
        jack->position.x += jack->velocityX;
        jack->position.y += jack->velocityY;
        jack->animTimer  += dt;

        if (jack->position.x < -150.0f ||
            jack->position.x > worldW + 150.0f ||
            jack->position.y < -300.0f          ||
            jack->position.y > worldH + 300.0f  ||
            jack->animTimer > 1.5f)
        {
            jack->state = JACK_STATE_GONE;
        }
        return;
    }

    if (jack->state == JACK_STATE_GONE) return;

    /* ============================================================
     *  Ladder detection & entry
     * ============================================================ */
    bool jumpPressed = IsKeyPressed(controls->jump) ||
                       IsKeyPressed(controls->up);

    if (!jack->climbing) {
        int idx = FindOverlappingLadder(jack, ladders, 2.0f, 2.0f);
        if (idx >= 0) {
            Rectangle lb    = ladders[idx].bounds;
            float     feetY = Jack_Bottom(jack);

            /* If the player's feet are right at the top edge of the
             * ladder and they are NOT pressing down, don't grab it.
             * This lets them stand on top without re-grabbing. */
            bool atTop        = (feetY <= lb.y + 3.0f);
            bool pressingDown = IsKeyDown(controls->down);

            if (!atTop || pressingDown) {
                jack->ladderIndex = idx;
                jack->onLadder    = true;
            } else {
                jack->onLadder    = false;
                jack->ladderIndex = -1;
            }
        } else {
            jack->onLadder    = false;
            jack->ladderIndex = -1;
        }
    } else {
        if (jack->ladderIndex < 0 || !ladders[jack->ladderIndex].active) {
            jack->climbing    = false;
            jack->onLadder    = false;
            jack->ladderIndex = -1;
        } else {
            jack->onLadder = true;
        }
    }

    /* Enter climbing on up/down press when on a ladder */
    if (!jack->climbing && jack->onLadder) {
        if (IsKeyDown(controls->up) || IsKeyDown(controls->down)) {
            jack->climbing  = true;
            jack->onGround  = false;
            jack->velocityY = 0.0f;
        }
    }

    /* ============================================================
     *  Climbing state
     * ============================================================ */
    if (jack->climbing && jack->ladderIndex >= 0) {
        const Ladder *L = &ladders[jack->ladderIndex];
        Rectangle lb = L->bounds;

        bool leftPressed  = IsKeyDown(controls->left);
        bool rightPressed = IsKeyDown(controls->right);

        if (leftPressed || rightPressed) {
            jack->climbing    = false;
            jack->onLadder    = false;
            jack->ladderIndex = -1;
        } else if (jumpPressed) {
            jack->climbing    = false;
            jack->onLadder    = false;
            jack->ladderIndex = -1;
            jack->velocityY   = -jack->jumpForce;
            jack->onGround    = false;
        } else {
            /* Snap X to ladder centre */
            jack->position.x = lb.x + lb.width * 0.5f;

            float climb = 0.0f;
            if (IsKeyDown(controls->up))   climb -= LADDER_CLIMB_SPEED;
            if (IsKeyDown(controls->down)) climb += LADDER_CLIMB_SPEED;

            jack->position.y += climb * dt;
            jack->velocityY   = 0.0f;
            jack->onGround    = false;

            float feetY = Jack_Bottom(jack);

            /* -------- Reached the TOP of the ladder (moving up) -------- */
            if (climb < 0.0f && feetY <= lb.y + 2.0f) {
                /* Try to continue onto a ladder directly above */
                int nextIdx = -1;
                for (int i = 0; i < MAX_LADDERS; i++) {
                    if (i == jack->ladderIndex) continue;
                    if (!ladders[i].active) continue;
                    Rectangle nb = ladders[i].bounds;

                    if (jack->position.x < nb.x - 6.0f) continue;
                    if (jack->position.x > nb.x + nb.width + 6.0f) continue;
                    if (nb.y >= lb.y - 1.0f) continue;

                    float nbBottom = nb.y + nb.height;
                    if (nbBottom < lb.y - 4.0f) continue;

                    nextIdx = i;
                    break;
                }

                if (nextIdx >= 0) {
                    jack->ladderIndex = nextIdx;
                } else {
                    /* No continuation — this is the true top.
                     * Snap player to stand on top and stop climbing. */
                    jack->position.y  = lb.y - jack->radiusY;
                    jack->climbing    = false;
                    jack->onLadder    = false;
                    jack->ladderIndex = -1;
                    jack->onGround    = true;
                    jack->velocityY   = 0.0f;
                    return;
                }
            }

            /* -------- Reached the BOTTOM of the ladder (moving down) -------- */
            float bottom = lb.y + lb.height;
            if (climb > 0.0f && feetY >= bottom - 2.0f) {
                int nextIdx = -1;
                for (int i = 0; i < MAX_LADDERS; i++) {
                    if (i == jack->ladderIndex) continue;
                    if (!ladders[i].active) continue;
                    Rectangle nb = ladders[i].bounds;

                    if (jack->position.x < nb.x - 6.0f) continue;
                    if (jack->position.x > nb.x + nb.width + 6.0f) continue;
                    if (nb.y <= lb.y + lb.height + 1.0f) continue;
                    if (nb.y > lb.y + lb.height + 4.0f) continue;

                    nextIdx = i;
                    break;
                }

                if (nextIdx >= 0) {
                    jack->ladderIndex = nextIdx;
                } else {
                    jack->position.y = bottom - jack->radiusY;
                    jack->onGround   = true;
                }
            }

            /* Animation timers */
            jack->animTimer += dt;
            if (jack->invulnTimer > 0.0f) {
                jack->invulnTimer -= dt;
                if (jack->invulnTimer < 0.0f) jack->invulnTimer = 0.0f;
            }
            if (jack->shootTimer > 0.0f) {
                jack->shootTimer -= dt;
                if (jack->shootTimer < 0.0f) jack->shootTimer = 0.0f;
            }

            if (jack->currentAnim != JACK_ANIM_WALK) {
                jack->currentAnim = JACK_ANIM_WALK;
                jack->animTimer   = 0.0f;
            }
            return;
        }
    }

    /* ============================================================
     *  Normal alive behaviour
     * ============================================================ */

    bool moving = false;

    if (IsKeyDown(controls->left)) {
        jack->position.x -= jack->moveSpeed;
        jack->facingDir = -1.0f;
        moving = true;
    }
    if (IsKeyDown(controls->right)) {
        jack->position.x += jack->moveSpeed;
        jack->facingDir = 1.0f;
        moving = true;
    }

    /* Horizontal platform collision — static platforms */
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

        bool wasOverlappingX =
            (prevPosition.x - jack->radiusX) < (p.x + p.width) &&
            (prevPosition.x + jack->radiusX) >  p.x;
        if (wasOverlappingX) continue;

        float platCX = p.x + p.width * 0.5f;
        if (jack->position.x < platCX) {
            jack->position.x = p.x - jack->radiusX;
        } else {
            jack->position.x = p.x + p.width + jack->radiusX;
        }
    }

    /* World X clamp */
    if (jack->position.x < jack->radiusX)
        jack->position.x = jack->radiusX;
    if (jack->position.x > worldW - jack->radiusX)
        jack->position.x = worldW - jack->radiusX;

    /* Jump */
    if (jumpPressed && jack->onGround) {
        jack->velocityY = -jack->jumpForce;
        jack->onGround  = false;
    }

    /* Gravity */
    jack->velocityY += jack->gravity;

    float beforeBottomMove = Jack_Bottom(jack);
    jack->position.y += jack->velocityY;
    bool landed = false;

    /* ---------- One-way landing — STATIC platforms ---------- */
    if (jack->velocityY >= 0.0f) {
        for (int i = 0; i < MAX_PLATFORMS; i++) {
            if (!platforms[i].active) continue;
            Rectangle p = platforms[i].bounds;

            if (jack->position.x < p.x)             continue;
            if (jack->position.x > p.x + p.width)   continue;

            if (beforeBottomMove <= p.y + 1.0f && Jack_Bottom(jack) >= p.y) {
                jack->position.y = p.y - jack->radiusY;
                jack->velocityY  = 0.0f;
                jack->onGround   = true;
                landed = true;
                break;
            }
        }

        /* ---------- One-way landing — MOVING platforms ---------- */
        if (!landed) {
            for (int i = 0; i < MAX_MOVING_PLATFORMS; i++) {
                if (!movingPlatforms[i].active) continue;
                Rectangle p = movingPlatforms[i].bounds;

                if (jack->position.x < p.x)             continue;
                if (jack->position.x > p.x + p.width)   continue;

                if (beforeBottomMove <= p.y + 1.0f && Jack_Bottom(jack) >= p.y) {
                    jack->position.y = p.y - jack->radiusY;
                    jack->velocityY  = 0.0f;
                    jack->onGround   = true;
                    landed = true;
                    break;
                }
            }
        }

        /* ---------- One-way landing — LADDER TOPS ----------
         * The top edge of every active ladder is walkable. After
         * `MergeStackedLadders` in game.c, stacked ladders are
         * already a single object with a single top, so no chain
         * detection is needed here. */
        if (!landed) {
            for (int i = 0; i < MAX_LADDERS; i++) {
                if (!ladders[i].active) continue;
                Rectangle lb = ladders[i].bounds;

                if (jack->position.x < lb.x)             continue;
                if (jack->position.x > lb.x + lb.width)  continue;

                if (beforeBottomMove <= lb.y + 1.0f &&
                    Jack_Bottom(jack) >= lb.y) {
                    jack->position.y = lb.y - jack->radiusY;
                    jack->velocityY  = 0.0f;
                    jack->onGround   = true;
                    landed = true;
                    break;
                }
            }
        }
    }
    if (!landed) jack->onGround = false;

    /* ---------- Chasm death ---------- */
    float killY = (float)worldH + 40.0f;
    if (jack->position.y > killY) {
        jack->lives = 0;
        jack->state = JACK_STATE_FLUNG;
        Jack_ApplyRandomFling(jack, 18.0f, 24.0f);
        return;
    }

    /* World ceiling */
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