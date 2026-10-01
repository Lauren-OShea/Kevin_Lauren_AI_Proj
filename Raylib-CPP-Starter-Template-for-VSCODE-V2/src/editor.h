#ifndef JACKFROST_EDITOR_H
#define JACKFROST_EDITOR_H

#include <stdbool.h>
#include "raylib.h"

#define EDITOR_WORLD_W       1200
#define EDITOR_WORLD_H         620
#define EDITOR_GRID             20

#define MAX_CUSTOM_LEVELS        5

#define EDITOR_MAX_PLATFORMS    40
#define EDITOR_MAX_LADDERS      16
#define EDITOR_MAX_SPIKES       40
#define EDITOR_MAX_JUMPPADS     20
#define EDITOR_MAX_MOVING        8
#define EDITOR_MAX_ENEMIES      30

typedef enum EditorTool {
    TOOL_PLATFORM = 0,
    TOOL_LADDER,
    TOOL_SPIKE,
    TOOL_JUMPPAD,
    TOOL_MOVING,
    TOOL_ENEMY,
    TOOL_SPAWN1,
    TOOL_SPAWN2,
    TOOL_COUNT
} EditorTool;

typedef struct EditorMoving {
    Rectangle startRect;
    Rectangle endRect;
} EditorMoving;

typedef struct EditorEnemy {
    Vector2 position;      /* foot position (bottom-center) */
    float   baseY;
    float   patrolLeft, patrolRight;
    int     type;          /* EnemyType */
} EditorEnemy;

typedef struct CustomLevel {
    bool      used;
    char      name[24];
    Vector2   spawn1, spawn2;

    int       nPlatforms;
    Rectangle platforms[EDITOR_MAX_PLATFORMS];

    int       nLadders;
    Rectangle ladders[EDITOR_MAX_LADDERS];

    int       nSpikes;
    Rectangle spikes[EDITOR_MAX_SPIKES];

    int       nJumppads;
    Rectangle jumppads[EDITOR_MAX_JUMPPADS];

    int       nMoving;
    EditorMoving moving[EDITOR_MAX_MOVING];

    int       nEnemies;
    EditorEnemy enemies[EDITOR_MAX_ENEMIES];
} CustomLevel;

/* Editor runtime state (lives in Game) */
typedef struct EditorState {
    bool       active;
    int        slot;       /* 0..MAX_CUSTOM_LEVELS-1 */
    EditorTool tool;
    int        enemyKind;  /* 0..4 */

    Camera2D   cam;

    bool       dragging;
    Vector2    dragStart;
    Vector2    dragEnd;

    bool       movingHasStart;
    Rectangle  movingStart;

    char       status[64];
    float      statusTimer;
} EditorState;

/* Save / load helpers */
void Editor_InitCustomLevel(CustomLevel *lvl);
void Editor_SaveAll        (const CustomLevel levels[MAX_CUSTOM_LEVELS]);
bool Editor_LoadAll        (      CustomLevel levels[MAX_CUSTOM_LEVELS]);

/* Enter the editor on a given slot */
void Editor_Enter(EditorState *e, int slot);

/* Full update+draw: called by game.c while state == GAME_STATE_EDITOR */
typedef struct Game Game;
void Editor_Update(Game *game);
void Editor_Draw  (const Game *game);

#endif /* JACKFROST_EDITOR_H */