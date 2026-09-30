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
    int left;
    int right;
    int up;
    int down;
    int jump;   /* dedicated jump key */
} JackControls;

typedef enum JackAnim {
    JACK_ANIM_IDLE = 0,
    JACK_ANIM_WALK,
    JACK_ANIM_JUMP,
    JACK_ANIM_SHOOT
} JackAnim;

typedef enum JackState {
    JACK_STATE_ALIVE = 0,
    JACK_STATE_FLUNG,
    JACK_STATE_GONE
} JackState;

typedef struct PlayerSprites {
    Texture2D tex   [JACK_ANIM_COUNT];
    int       frameW[JACK_ANIM_COUNT];
    int       frameH[JACK_ANIM_COUNT];
    bool      loaded[JACK_ANIM_COUNT];
} PlayerSprites;

typedef struct Jack {
    Vector2 position;
    Vector2 spawnPosition;   

    float   radiusX;
    float   radiusY;

    float   velocityX;
    float   velocityY;
    float   moveSpeed;
    float   jumpForce;
    float   gravity;
    float   facingDir;
    bool    onGround;

    /* ----- Ladder state ----- */
    bool    onLadder;      /* currently overlapping a ladder */
    bool    climbing;      /* attached and climbing */
    int     ladderIndex;   /* index into ladders[] while attached, else -1 */

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
void Jack_Update(Jack *jack,
                 const Platform platforms[MAX_PLATFORMS],
                 const Ladder   ladders[MAX_LADDERS],
                 const JackControls *controls, int worldW, int worldH);
void Jack_Draw  (const Jack *jack, const PlayerSprites *sprites);

void Jack_TriggerShoot(Jack *jack);
void Jack_Hit         (Jack *jack);
bool Jack_IsPlayable  (const Jack *jack);
bool Jack_IsVulnerable(const Jack *jack);
bool Jack_IsInvulnerable(const Jack *jack);

#endif /* JACK_H */