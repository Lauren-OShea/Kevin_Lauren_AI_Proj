#ifndef JACK_H
#define JACK_H

#include "raylib.h"
#include "platform.h"

#define JACK_ANIM_COUNT     4
#define SPRITE_SCALE        1.5f
#define JACK_FEET_OFFSET    12.0f
#define JACK_MAX_LIVES      3
#define JACK_INVULN_TIME    2.0f

typedef struct JackControls {
    int keyLeft;
    int keyRight;
    int keyJump;
} JackControls;

typedef enum JackAnim {
    JACK_ANIM_IDLE = 0,
    JACK_ANIM_WALK,
    JACK_ANIM_JUMP,
    JACK_ANIM_SHOOT
} JackAnim;

typedef enum JackState {
    JACK_STATE_ALIVE = 0,
    JACK_STATE_FLUNG,   /* lost last life — flying off screen */
    JACK_STATE_GONE     /* off screen, out of the game */
} JackState;

typedef struct PlayerSprites {
    Texture2D tex   [JACK_ANIM_COUNT];
    int       frameW[JACK_ANIM_COUNT];
    int       frameH[JACK_ANIM_COUNT];
    bool      loaded[JACK_ANIM_COUNT];
} PlayerSprites;

typedef struct Jack {
    Vector2 position;
    float   radiusX;
    float   radiusY;

    float   velocityX;   /* only used during FLUNG */
    float   velocityY;
    float   moveSpeed;
    float   jumpForce;
    float   gravity;
    float   facingDir;
    bool    onGround;

    JackState state;
    JackAnim  currentAnim;
    float     animTimer;
    float     shootTimer;
    float     invulnTimer;
    int       lives;

    Color     tintColor;
    float     feetOffsetY;
} Jack;

void PlayerSprites_Load  (PlayerSprites *sprites);
void PlayerSprites_Unload(PlayerSprites *sprites);

void Jack_Init  (Jack *jack, Vector2 startPos, Color tintColor);
void Jack_Update(Jack *jack, const Platform platforms[MAX_PLATFORMS],
                 const JackControls *controls);
void Jack_Draw  (const Jack *jack, const PlayerSprites *sprites);

void Jack_TriggerShoot(Jack *jack);
void Jack_Hit         (Jack *jack);
bool Jack_IsPlayable  (const Jack *jack);   /* alive → can move/shoot */
bool Jack_IsVulnerable(const Jack *jack);   /* enemies can hurt them  */
bool Jack_IsInvulnerable(const Jack *jack); /* should blink           */

#endif /* JACK_H */