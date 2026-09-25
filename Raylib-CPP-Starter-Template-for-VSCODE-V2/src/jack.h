#ifndef JACK_H
#define JACK_H

#include "raylib.h"
#include "platform.h"

#define JACK_ANIM_COUNT       7
#define SPRITE_SCALE          1.5f
#define JACK_DOWN_DURATION    1.0f
#define JACK_FEET_OFFSET      12.0f
#define JACK_POST_GETUP_INVULN 2.0f   /* seconds of blinking after recovery */

typedef struct JackControls {
    int keyLeft;
    int keyRight;
    int keyJump;
} JackControls;

typedef enum JackAnim {
    JACK_ANIM_IDLE = 0,
    JACK_ANIM_WALK,
    JACK_ANIM_JUMP,
    JACK_ANIM_SHOOT,
    JACK_ANIM_KNOCKED_DOWN,
    JACK_ANIM_DOWN,
    JACK_ANIM_GET_UP
} JackAnim;

typedef enum JackState {
    JACK_STATE_NORMAL = 0,
    JACK_STATE_SHOOTING,
    JACK_STATE_KNOCKED_DOWN,
    JACK_STATE_DOWN,
    JACK_STATE_GETTING_UP
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

    float   velocityY;
    float   moveSpeed;
    float   jumpForce;
    float   gravity;
    float   facingDir;
    bool    onGround;

    JackState state;
    JackAnim  currentAnim;
    float     animTimer;
    float     stateTimer;
    float     invulnTimer;      /* NEW — post-getup grace period */

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
void Jack_Knockdown   (Jack *jack);
bool Jack_IsVulnerable   (const Jack *jack);   /* can enemies hurt them? */
bool Jack_IsInvulnerable (const Jack *jack);   /* should we blink them?  */

#endif /* JACK_H */