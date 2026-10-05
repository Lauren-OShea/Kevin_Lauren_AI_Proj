#include "game.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/* ============================================================
 *  Global Y offset
 * ============================================================ */

#define LEVEL_Y_SHIFT 12

/* Forward declarations for static helpers defined later */
static void BuildLadders(Game *game);
static bool LadderBlockedByPlatform(const Game *game,
                                    float x0, float x1,
                                    float topY, float bottomY);
static void MarkFreezableTiles(Game *game);

/* ============================================================
 *  Level data
 * ============================================================ */

#define MAX_LVL_PLATFORMS 30
#define MAX_LVL_ENEMIES   25

typedef struct LvlEnemy {
    int  platformIdx;
    int  type;        /* EnemyType */
} LvlEnemy;

typedef struct LevelDef {
    const char *name;
    int         worldW, worldH;
    Vector2     spawn1, spawn2;
    Rectangle   exitZone;
    int         nPlatforms;
    Rectangle   platforms[MAX_LVL_PLATFORMS];
    int         nEnemies;
    LvlEnemy    enemies[MAX_LVL_ENEMIES];
} LevelDef;

static const LevelDef LEVELS[MAX_LEVELS] = {

/* ==================== Zone 1 — 900 × 500 ==================== */

{ "First Crack", 900, 500, {80,400}, {140,400}, {0,0,0,0},
  9, {
      {   0, 440, 300, 60 },
      { 400, 440, 500, 60 },
      { 140, 360, 120, 24 },
      { 320, 360, 120, 24 },
      { 500, 360, 120, 24 },
      { 680, 360, 120, 24 },
      { 240, 280, 120, 24 },
      { 440, 280, 120, 24 },
      { 640, 280, 120, 24 },
  },
  1, { {3,ENEMY_WALKER} } },

{ "Stepping Stones", 900, 500, {80,400}, {140,400}, {0,0,0,0},
 10, {
      {   0, 440, 250, 60 },
      { 340, 440, 240, 60 },
      { 660, 440, 240, 60 },
      { 140, 360, 120, 24 },
      { 340, 360, 120, 24 },
      { 540, 360, 120, 24 },
      { 740, 360, 120, 24 },
      { 200, 280, 120, 24 },
      { 440, 280, 120, 24 },
      { 660, 280, 120, 24 },
  },
  2, { {3,ENEMY_WALKER}, {6,ENEMY_WALKER} } },

{ "Zigzag Cracks", 900, 500, {80,400}, {140,400}, {0,0,0,0},
 12, {
      {   0, 440, 200, 60 },
      { 280, 440, 180, 60 },
      { 540, 440, 180, 60 },
      { 800, 440, 100, 60 },
      { 100, 360, 120, 24 },
      { 300, 360, 120, 24 },
      { 500, 360, 120, 24 },
      { 700, 360, 120, 24 },
      { 200, 280, 120, 24 },
      { 400, 280, 120, 24 },
      { 600, 280, 120, 24 },
      { 400, 200, 120, 24 },
  },
  3, { {4,ENEMY_WALKER}, {6,ENEMY_SHOOTER}, {11,ENEMY_WALKER} } },

{ "Wide Gap", 900, 500, {80,400}, {140,400}, {0,0,0,0},
 11, {
      {   0, 440, 300, 60 },
      { 500, 440, 400, 60 },
      { 180, 360, 120, 24 },
      { 400, 360, 140, 24 },
      { 600, 360, 140, 24 },
      { 300, 280, 120, 24 },
      { 500, 280, 120, 24 },
      { 400, 200, 120, 24 },
      {  60, 360, 100, 24 },
      { 740, 360, 100, 24 },
      { 300, 200, 100, 24 },
  },
  3, { {3,ENEMY_WALKER}, {6,ENEMY_SHOOTER}, {7,ENEMY_WALKER} } },

{ "Watchtower", 900, 500, {80,400}, {140,400}, {0,0,0,0},
 14, {
      {   0, 440, 250, 60 },
      { 350, 440, 250, 60 },
      { 700, 440, 200, 60 },
      { 140, 360, 120, 24 },
      { 340, 360, 120, 24 },
      { 540, 360, 120, 24 },
      { 740, 360, 120, 24 },
      { 240, 280, 120, 24 },
      { 440, 280, 120, 24 },
      { 640, 280, 120, 24 },
      { 340, 200, 120, 24 },
      { 540, 200, 120, 24 },
      {  60, 360, 100, 24 },
      { 810, 360, 100, 24 },
  },
  4, { {3,ENEMY_WALKER}, {5,ENEMY_DASHER}, {8,ENEMY_SHOOTER}, {10,ENEMY_WALKER} } },

/* ==================== Zone 2 — 1000 × 540 ==================== */

{ "Double Pit", 1000, 540, {80,440}, {140,440}, {0,0,0,0},
 15, {
      {   0, 480, 240, 60 },
      { 400, 480, 240, 60 },
      { 760, 480, 240, 60 },
      {  60, 400, 110, 24 },
      { 200, 400, 110, 24 },
      { 340, 400, 110, 24 },
      { 480, 400, 110, 24 },
      { 620, 400, 110, 24 },
      { 760, 400, 110, 24 },
      { 120, 320, 110, 24 },
      { 260, 320, 110, 24 },
      { 400, 320, 110, 24 },
      { 540, 320, 110, 24 },
      { 680, 320, 110, 24 },
      { 820, 320, 110, 24 },
  },
  5, { {3,ENEMY_DASHER}, {5,ENEMY_SHOOTER}, {8,ENEMY_WALKER},
       {10,ENEMY_DASHER}, {13,ENEMY_SHOOTER} } },

{ "Pillars of Ice", 1000, 540, {80,440}, {140,440}, {0,0,0,0},
 17, {
      {   0, 480, 1000, 60 },
      {  60, 400, 100, 24 },
      {  60, 320, 100, 24 },
      { 240, 400, 100, 24 },
      { 240, 320, 100, 24 },
      { 420, 400, 100, 24 },
      { 420, 320, 100, 24 },
      { 600, 400, 100, 24 },
      { 600, 320, 100, 24 },
      { 780, 400, 100, 24 },
      { 780, 320, 100, 24 },
      { 900, 400, 100, 24 },
      { 340, 240, 120, 24 },
      { 540, 240, 120, 24 },
      { 200, 240, 100, 24 },
      { 700, 240, 100, 24 },
      { 440, 160, 120, 24 },
  },
  6, { {1,ENEMY_WALKER}, {2,ENEMY_SHOOTER}, {4,ENEMY_UNSTUNNABLE},
       {7,ENEMY_DASHER}, {10,ENEMY_SHOOTER}, {12,ENEMY_WALKER} } },

{ "Grand Canyon", 1000, 540, {80,440}, {140,440}, {0,0,0,0},
 16, {
      {   0, 480, 200, 60 },
      { 340, 480, 320, 60 },
      { 800, 480, 200, 60 },
      {  40, 400, 100, 24 },
      { 160, 400, 100, 24 },
      { 280, 400, 100, 24 },
      { 400, 400, 100, 24 },
      { 520, 400, 100, 24 },
      { 640, 400, 100, 24 },
      { 760, 400, 100, 24 },
      { 880, 400, 100, 24 },
      { 100, 320, 100, 24 },
      { 240, 320, 100, 24 },
      { 380, 320, 100, 24 },
      { 520, 320, 100, 24 },
      { 660, 320, 100, 24 },
  },
  6, { {1,ENEMY_DASHER}, {3,ENEMY_SHOOTER}, {6,ENEMY_UNSTUNNABLE},
       {9,ENEMY_DASHER}, {11,ENEMY_SHOOTER}, {14,ENEMY_WALKER} } },

{ "Frozen Falls", 1000, 540, {80,440}, {140,440}, {0,0,0,0},
 18, {
      {   0, 480, 260, 60 },
      { 400, 480, 220, 60 },
      { 760, 480, 240, 60 },
      {  60, 400, 110, 24 },
      { 200, 400, 110, 24 },
      { 340, 400, 110, 24 },
      { 480, 400, 110, 24 },
      { 620, 400, 110, 24 },
      { 760, 400, 110, 24 },
      { 120, 320, 110, 24 },
      { 260, 320, 110, 24 },
      { 400, 320, 110, 24 },
      { 540, 320, 110, 24 },
      { 680, 320, 110, 24 },
      { 820, 320, 110, 24 },
      { 240, 240, 180, 24 },
      { 500, 240, 180, 24 },
      { 760, 240, 180, 24 },
  },
  7, { {3,ENEMY_DASHER}, {4,ENEMY_SHOOTER}, {6,ENEMY_UNSTUNNABLE},
       {8,ENEMY_DASHER}, {10,ENEMY_SHOOTER}, {12,ENEMY_WALKER}, {16,ENEMY_DASHER} } },

{ "Chasm", 1000, 540, {80,440}, {140,440}, {0,0,0,0},
 15, {
      {   0, 480, 240, 60 },
      { 340, 480, 340, 60 },
      { 780, 480, 220, 60 },
      {  40, 400, 110, 24 },
      { 160, 320, 110, 24 },
      { 260, 400, 110, 24 },
      { 380, 320, 110, 24 },
      { 500, 400, 110, 24 },
      { 620, 320, 110, 24 },
      { 740, 400, 110, 24 },
      { 860, 320, 110, 24 },
      { 400, 240, 200, 24 },
      { 700, 240, 200, 24 },
      { 200, 160, 200, 24 },
      { 560, 160, 200, 24 },
  },
  9, { {3,ENEMY_DASHER}, {4,ENEMY_SHOOTER}, {5,ENEMY_WALKER},
       {6,ENEMY_DASHER}, {7,ENEMY_SHOOTER}, {9,ENEMY_UNSTUNNABLE},
       {11,ENEMY_DASHER}, {13,ENEMY_SHOOTER}, {14,ENEMY_WALKER} } },

/* ==================== Zone 3 — 1100 × 580 ==================== */

{ "Broken Bridge", 1100, 580, {80,480}, {140,480}, {0,0,0,0},
 18, {
      {   0, 520, 220, 60 },
      { 320, 520, 160, 60 },
      { 580, 520, 160, 60 },
      { 840, 520, 260, 60 },
      {  60, 440, 100, 24 },
      { 200, 440, 100, 24 },
      { 340, 440, 100, 24 },
      { 480, 440, 100, 24 },
      { 620, 440, 100, 24 },
      { 760, 440, 100, 24 },
      { 900, 440, 100, 24 },
      { 120, 360, 100, 24 },
      { 260, 360, 100, 24 },
      { 400, 360, 100, 24 },
      { 540, 360, 100, 24 },
      { 680, 360, 100, 24 },
      { 820, 360, 100, 24 },
      { 960, 360, 100, 24 },
  },
  8, { {4,ENEMY_DASHER}, {6,ENEMY_FLYER}, {8,ENEMY_UNSTUNNABLE},
       {10,ENEMY_DASHER}, {12,ENEMY_SHOOTER}, {14,ENEMY_FLYER},
       {16,ENEMY_DASHER}, {17,ENEMY_SHOOTER} } },

{ "Triple Threat", 1100, 580, {80,480}, {140,480}, {0,0,0,0},
 19, {
      {   0, 520, 200, 60 },
      { 300, 520, 200, 60 },
      { 600, 520, 200, 60 },
      { 900, 520, 200, 60 },
      {  40, 440, 100, 24 },
      { 180, 440, 100, 24 },
      { 320, 440, 100, 24 },
      { 460, 440, 100, 24 },
      { 600, 440, 100, 24 },
      { 740, 440, 100, 24 },
      { 880, 440, 100, 24 },
      {1020, 440,  80, 24 },
      { 100, 360, 100, 24 },
      { 240, 360, 100, 24 },
      { 380, 360, 100, 24 },
      { 520, 360, 100, 24 },
      { 660, 360, 100, 24 },
      { 800, 360, 100, 24 },
      { 940, 360, 100, 24 },
  },
  9, { {4,ENEMY_DASHER}, {5,ENEMY_FLYER}, {7,ENEMY_UNSTUNNABLE},
       {8,ENEMY_DASHER}, {10,ENEMY_SHOOTER}, {11,ENEMY_FLYER},
       {13,ENEMY_DASHER}, {15,ENEMY_SHOOTER}, {17,ENEMY_UNSTUNNABLE} } },

{ "Labyrinth", 1100, 580, {80,480}, {140,480}, {0,0,0,0},
 21, {
      {   0, 520, 200, 60 },
      { 300, 520, 200, 60 },
      { 600, 520, 200, 60 },
      { 900, 520, 200, 60 },
      {  40, 440, 100, 24 },
      { 180, 440, 100, 24 },
      { 320, 440, 100, 24 },
      { 460, 440, 100, 24 },
      { 600, 440, 100, 24 },
      { 740, 440, 100, 24 },
      { 880, 440, 100, 24 },
      {1020, 440,  80, 24 },
      {  40, 360, 100, 24 },
      { 180, 360, 100, 24 },
      { 320, 360, 100, 24 },
      { 460, 360, 100, 24 },
      { 600, 360, 100, 24 },
      { 740, 360, 100, 24 },
      { 880, 360, 100, 24 },
      { 240, 280, 200, 24 },
      { 660, 280, 200, 24 },
  },
  10, { {4,ENEMY_FLYER}, {5,ENEMY_DASHER}, {6,ENEMY_UNSTUNNABLE},
        {8,ENEMY_FLYER}, {9,ENEMY_DASHER}, {11,ENEMY_SHOOTER},
        {12,ENEMY_FLYER}, {14,ENEMY_DASHER}, {16,ENEMY_UNSTUNNABLE},
        {18,ENEMY_SHOOTER} } },

{ "Ice Cathedral", 1100, 580, {80,480}, {140,480}, {0,0,0,0},
 22, {
      {   0, 520, 180, 60 },
      { 280, 520, 180, 60 },
      { 560, 520, 180, 60 },
      { 840, 520, 260, 60 },
      {  40, 440, 100, 24 },
      { 180, 440, 100, 24 },
      { 320, 440, 100, 24 },
      { 460, 440, 100, 24 },
      { 600, 440, 100, 24 },
      { 740, 440, 100, 24 },
      { 880, 440, 100, 24 },
      { 100, 360, 100, 24 },
      { 240, 360, 100, 24 },
      { 380, 360, 100, 24 },
      { 520, 360, 100, 24 },
      { 660, 360, 100, 24 },
      { 800, 360, 100, 24 },
      { 940, 360, 100, 24 },
      { 300, 280, 120, 24 },
      { 500, 280, 120, 24 },
      { 700, 280, 120, 24 },
      { 400, 200, 300, 24 },
  },
  11, { {4,ENEMY_FLYER}, {5,ENEMY_DASHER}, {6,ENEMY_UNSTUNNABLE},
        {7,ENEMY_FLYER}, {9,ENEMY_DASHER}, {10,ENEMY_SHOOTER},
        {12,ENEMY_FLYER}, {14,ENEMY_DASHER}, {16,ENEMY_UNSTUNNABLE},
        {18,ENEMY_SHOOTER}, {21,ENEMY_FLYER} } },

{ "Frozen Rapids", 1100, 580, {80,480}, {140,480}, {0,0,0,0},
 22, {
      {   0, 520, 200, 60 },
      { 300, 520, 160, 60 },
      { 560, 520, 160, 60 },
      { 820, 520, 280, 60 },
      {  60, 440, 100, 24 },
      { 200, 440, 100, 24 },
      { 340, 440, 100, 24 },
      { 480, 440, 100, 24 },
      { 620, 440, 100, 24 },
      { 760, 440, 100, 24 },
      { 900, 440, 100, 24 },
      { 120, 360, 100, 24 },
      { 260, 360, 100, 24 },
      { 400, 360, 100, 24 },
      { 540, 360, 100, 24 },
      { 680, 360, 100, 24 },
      { 820, 360, 100, 24 },
      { 960, 360, 100, 24 },
      { 200, 280, 140, 24 },
      { 420, 280, 140, 24 },
      { 640, 280, 140, 24 },
      { 860, 280, 140, 24 },
  },
  12, { {4,ENEMY_FLYER}, {5,ENEMY_DASHER}, {6,ENEMY_UNSTUNNABLE},
        {7,ENEMY_FLYER}, {8,ENEMY_DASHER}, {9,ENEMY_SHOOTER},
        {10,ENEMY_FLYER}, {12,ENEMY_DASHER}, {14,ENEMY_UNSTUNNABLE},
        {16,ENEMY_FLYER}, {18,ENEMY_DASHER}, {20,ENEMY_SHOOTER} } },

/* ==================== Zone 4 — 1200 × 620 ==================== */

{ "Shattered Ice", 1200, 620, {80,520}, {140,520}, {0,0,0,0},
 24, {
      {   0, 560, 200, 60 },
      { 300, 560, 180, 60 },
      { 580, 560, 180, 60 },
      { 860, 560, 340, 60 },
      {  20, 480, 100, 24 },
      { 160, 480, 100, 24 },
      { 300, 480, 100, 24 },
      { 440, 480, 100, 24 },
      { 580, 480, 100, 24 },
      { 720, 480, 100, 24 },
      { 860, 480, 100, 24 },
      {1000, 480, 100, 24 },
      {  80, 400, 100, 24 },
      { 220, 400, 100, 24 },
      { 360, 400, 100, 24 },
      { 500, 400, 100, 24 },
      { 640, 400, 100, 24 },
      { 780, 400, 100, 24 },
      { 920, 400, 100, 24 },
      {1060, 400, 100, 24 },
      { 200, 320, 140, 24 },
      { 440, 320, 140, 24 },
      { 680, 320, 140, 24 },
      { 920, 320, 140, 24 },
  },
  13, { {4,ENEMY_FLYER}, {5,ENEMY_DASHER}, {6,ENEMY_UNSTUNNABLE},
        {7,ENEMY_FLYER}, {8,ENEMY_DASHER}, {9,ENEMY_SHOOTER},
        {10,ENEMY_FLYER}, {12,ENEMY_DASHER}, {14,ENEMY_UNSTUNNABLE},
        {16,ENEMY_FLYER}, {18,ENEMY_DASHER}, {20,ENEMY_SHOOTER},
        {23,ENEMY_FLYER} } },

{ "The Gauntlet", 1200, 620, {80,520}, {140,520}, {0,0,0,0},
 25, {
      {   0, 560, 180, 60 },
      { 280, 560, 160, 60 },
      { 540, 560, 160, 60 },
      { 800, 560, 160, 60 },
      {1060, 560, 140, 60 },
      {  20, 480, 100, 24 },
      { 160, 480, 100, 24 },
      { 300, 480, 100, 24 },
      { 440, 480, 100, 24 },
      { 580, 480, 100, 24 },
      { 720, 480, 100, 24 },
      { 860, 480, 100, 24 },
      {1000, 480, 100, 24 },
      {  80, 400, 100, 24 },
      { 220, 400, 100, 24 },
      { 360, 400, 100, 24 },
      { 500, 400, 100, 24 },
      { 640, 400, 100, 24 },
      { 780, 400, 100, 24 },
      { 920, 400, 100, 24 },
      {1060, 400, 100, 24 },
      { 160, 320, 120, 24 },
      { 400, 320, 120, 24 },
      { 640, 320, 120, 24 },
      { 880, 320, 120, 24 },
  },
  14, { {5,ENEMY_FLYER}, {6,ENEMY_DASHER}, {7,ENEMY_UNSTUNNABLE},
        {8,ENEMY_FLYER}, {9,ENEMY_DASHER}, {10,ENEMY_SHOOTER},
        {11,ENEMY_FLYER}, {12,ENEMY_DASHER}, {13,ENEMY_UNSTUNNABLE},
        {15,ENEMY_FLYER}, {17,ENEMY_DASHER}, {19,ENEMY_SHOOTER},
        {21,ENEMY_FLYER}, {23,ENEMY_DASHER} } },

{ "Glacial Divide", 1200, 620, {80,520}, {140,520}, {0,0,0,0},
 26, {
      {   0, 560, 200, 60 },
      { 340, 560, 200, 60 },
      { 680, 560, 200, 60 },
      {1020, 560, 180, 60 },
      {  40, 480, 100, 24 },
      { 180, 480, 100, 24 },
      { 320, 480, 100, 24 },
      { 460, 480, 100, 24 },
      { 600, 480, 100, 24 },
      { 740, 480, 100, 24 },
      { 880, 480, 100, 24 },
      {1020, 480, 100, 24 },
      { 100, 400, 100, 24 },
      { 240, 400, 100, 24 },
      { 380, 400, 100, 24 },
      { 520, 400, 100, 24 },
      { 660, 400, 100, 24 },
      { 800, 400, 100, 24 },
      { 940, 400, 100, 24 },
      {1060, 400, 100, 24 },
      { 200, 320, 120, 24 },
      { 420, 320, 120, 24 },
      { 640, 320, 120, 24 },
      { 860, 320, 120, 24 },
      { 340, 240, 140, 24 },
      { 660, 240, 140, 24 },
  },
  15, { {4,ENEMY_FLYER}, {5,ENEMY_DASHER}, {6,ENEMY_UNSTUNNABLE},
        {7,ENEMY_FLYER}, {8,ENEMY_DASHER}, {9,ENEMY_SHOOTER},
        {10,ENEMY_FLYER}, {12,ENEMY_DASHER}, {14,ENEMY_UNSTUNNABLE},
        {16,ENEMY_FLYER}, {18,ENEMY_DASHER}, {20,ENEMY_SHOOTER},
        {22,ENEMY_FLYER}, {24,ENEMY_UNSTUNNABLE}, {25,ENEMY_DASHER} } },

{ "Frozen Labyrinth", 1200, 620, {80,520}, {140,520}, {0,0,0,0},
 27, {
      {   0, 560, 200, 60 },
      { 320, 560, 180, 60 },
      { 620, 560, 180, 60 },
      { 920, 560, 280, 60 },
      {  20, 480, 100, 24 },
      { 160, 480, 100, 24 },
      { 300, 480, 100, 24 },
      { 440, 480, 100, 24 },
      { 580, 480, 100, 24 },
      { 720, 480, 100, 24 },
      { 860, 480, 100, 24 },
      {1000, 480, 100, 24 },
      {  20, 400, 100, 24 },
      { 160, 400, 100, 24 },
      { 300, 400, 100, 24 },
      { 440, 400, 100, 24 },
      { 580, 400, 100, 24 },
      { 720, 400, 100, 24 },
      { 860, 400, 100, 24 },
      {1000, 400, 100, 24 },
      {  20, 320, 100, 24 },
      { 160, 320, 100, 24 },
      { 300, 320, 100, 24 },
      { 440, 320, 100, 24 },
      { 580, 320, 100, 24 },
      { 720, 320, 100, 24 },
      { 860, 320, 100, 24 },
  },
  16, { {4,ENEMY_FLYER}, {5,ENEMY_DASHER}, {6,ENEMY_UNSTUNNABLE},
        {7,ENEMY_FLYER}, {8,ENEMY_DASHER}, {9,ENEMY_SHOOTER},
        {10,ENEMY_FLYER}, {12,ENEMY_DASHER}, {13,ENEMY_UNSTUNNABLE},
        {15,ENEMY_FLYER}, {16,ENEMY_DASHER}, {18,ENEMY_SHOOTER},
        {19,ENEMY_FLYER}, {21,ENEMY_DASHER}, {23,ENEMY_UNSTUNNABLE},
        {25,ENEMY_FLYER} } },

{ "Absolute Zero", 1200, 620, {80,520}, {140,520}, {0,0,0,0},
 28, {
      {   0, 560, 180, 60 },
      { 300, 560, 160, 60 },
      { 580, 560, 160, 60 },
      { 860, 560, 340, 60 },
      {  20, 480,  95, 24 },
      { 135, 480,  95, 24 },
      { 250, 480,  95, 24 },
      { 365, 480,  95, 24 },
      { 480, 480,  95, 24 },
      { 595, 480,  95, 24 },
      { 710, 480,  95, 24 },
      { 825, 480,  95, 24 },
      { 940, 480,  95, 24 },
      {1055, 480, 120, 24 },
      {  20, 400,  95, 24 },
      { 135, 400,  95, 24 },
      { 250, 400,  95, 24 },
      { 365, 400,  95, 24 },
      { 480, 400,  95, 24 },
      { 595, 400,  95, 24 },
      { 710, 400,  95, 24 },
      { 825, 400,  95, 24 },
      { 940, 400,  95, 24 },
      {1055, 400, 120, 24 },
      {  60, 320, 110, 24 },
      { 260, 320, 110, 24 },
      { 460, 320, 110, 24 },
      { 660, 320, 110, 24 },
  },
  18, { {4,ENEMY_FLYER}, {5,ENEMY_DASHER}, {6,ENEMY_UNSTUNNABLE},
        {7,ENEMY_FLYER}, {8,ENEMY_DASHER}, {9,ENEMY_SHOOTER},
        {10,ENEMY_FLYER}, {11,ENEMY_DASHER}, {12,ENEMY_UNSTUNNABLE},
        {13,ENEMY_FLYER}, {14,ENEMY_DASHER}, {16,ENEMY_SHOOTER},
        {18,ENEMY_FLYER}, {20,ENEMY_DASHER}, {22,ENEMY_UNSTUNNABLE},
        {24,ENEMY_FLYER}, {25,ENEMY_DASHER}, {27,ENEMY_SHOOTER} } },
};

/* ============================================================
 *  Random entity placement
 * ============================================================ */

static void PlaceJumppadsRandomly(Game *game, const LevelDef *L) {
    for (int i = 0; i < L->nPlatforms; i++) {
        Rectangle plat = L->platforms[i];

        if (GetRandomValue(0, 99) >= 15) continue;
        if (plat.width < JUMPPAD_WIDTH + 20.0f) continue;

        float px = plat.x + 10.0f +
                   (float)GetRandomValue(0, (int)(plat.width - JUMPPAD_WIDTH - 20.0f));
        Rectangle padRect = { px, plat.y - JUMPPAD_HEIGHT,
                              JUMPPAD_WIDTH, JUMPPAD_HEIGHT };

        bool overlapsEnemy = false;
        for (int e = 0; e < MAX_ENEMIES; e++) {
            if (!game->enemies[e].active) continue;
            Rectangle er = Enemy_GetRect(&game->enemies[e]);
            er.x -= 8; er.width += 16;
            if (CheckCollisionRecs(padRect, er)) { overlapsEnemy = true; break; }
        }
        if (overlapsEnemy) continue;

        JumpPad_Add(game->jumppads, padRect);
    }
}

static void PlaceSpikesRandomly(Game *game, const LevelDef *L) {
    for (int i = 0; i < L->nPlatforms; i++) {
        Rectangle plat = L->platforms[i];

        if (GetRandomValue(0, 99) >= 25) continue;
        if (plat.width < SPIKE_WIDTH + 20.0f) continue;

        float sx = plat.x + 10.0f +
                   (float)GetRandomValue(0, (int)(plat.width - SPIKE_WIDTH - 20.0f));
        Rectangle spikeRect = { sx, plat.y - SPIKE_HEIGHT,
                                SPIKE_WIDTH, SPIKE_HEIGHT };

        bool tooClose = false;
        for (int e = 0; e < MAX_ENEMIES; e++) {
            if (!game->enemies[e].active) continue;
            Rectangle er = Enemy_GetRect(&game->enemies[e]);
            er.x      -= SPIKE_SAFE_MARGIN;
            er.y      -= SPIKE_SAFE_MARGIN;
            er.width  += SPIKE_SAFE_MARGIN * 2;
            er.height += SPIKE_SAFE_MARGIN * 2;
            if (CheckCollisionRecs(spikeRect, er)) { tooClose = true; break; }
        }
        if (tooClose) continue;
        if (Spike_OverlapsRect(game->spikes, spikeRect)) continue;

        bool overlapsPad = false;
        for (int j = 0; j < MAX_JUMPPADS; j++) {
            if (!game->jumppads[j].active) continue;
            if (CheckCollisionRecs(spikeRect, game->jumppads[j].bounds)) {
                overlapsPad = true;
                break;
            }
        }
        if (overlapsPad) continue;

        Spike_Add(game->spikes, spikeRect);
    }
}

static bool MovingPlatformPlacementSafe(const Game *game, const LevelDef *L,
                                        Rectangle swept)
{
    if (swept.x < 10.0f || swept.y < 10.0f) return false;
    if (swept.x + swept.width  > L->worldW - 10.0f) return false;
    if (swept.y + swept.height > L->worldH - 10.0f) return false;

    for (int i = 0; i < L->nPlatforms; i++) {
        Rectangle p  = L->platforms[i];
        Rectangle pe = { p.x - 8, p.y - 8, p.width + 16, p.height + 16 };
        if (CheckCollisionRecs(swept, pe)) return false;
    }

    for (int i = 0; i < MAX_MOVING_PLATFORMS; i++) {
        if (!game->movingPlatforms[i].active) continue;
        Rectangle s = MovingPlatform_GetSweptBox(&game->movingPlatforms[i]);
        s.x -= 12; s.y -= 12; s.width += 24; s.height += 24;
        if (CheckCollisionRecs(swept, s)) return false;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!game->enemies[i].active) continue;
        if (game->enemies[i].type != ENEMY_FLYER) continue;

        Rectangle er = {
            game->enemies[i].patrolLeft,
            game->enemies[i].baseY - ENEMY_FLY_AMPLITUDE - 8.0f,
            game->enemies[i].patrolRight - game->enemies[i].patrolLeft,
            ENEMY_FLY_AMPLITUDE * 2.0f + 16.0f
        };
        er.x -= 16; er.width += 32;
        if (CheckCollisionRecs(swept, er)) return false;
    }

    for (int i = 0; i < MAX_JUMPPADS; i++) {
        if (!game->jumppads[i].active) continue;
        if (CheckCollisionRecs(swept, game->jumppads[i].bounds)) return false;
    }
    for (int i = 0; i < MAX_SPIKES; i++) {
        if (!game->spikes[i].active) continue;
        if (CheckCollisionRecs(swept, game->spikes[i].bounds)) return false;
    }

    return true;
}

static void PlaceMovingPlatformsRandomly(Game *game, const LevelDef *L,
                                         int targetCount)
{
    int placed = 0;

    for (int attempt = 0; attempt < 300 && placed < targetCount; attempt++) {
        float w = (float)GetRandomValue(70, 100);
        float h = 16.0f;

        float x1 = (float)GetRandomValue(60, L->worldW - (int)w - 60);
        float y1 = (float)GetRandomValue(80, L->worldH - 200);

        int   axis = GetRandomValue(0, 2);
        float dx = 0, dy = 0;

        if (axis == 0) {
            dx = (float)GetRandomValue(40, 70);
            if (GetRandomValue(0, 1) == 0) dx = -dx;
        } else if (axis == 1) {
            dy = (float)GetRandomValue(30, 55);
            if (GetRandomValue(0, 1) == 0) dy = -dy;
        } else {
            dx = (float)GetRandomValue(30, 50);
            dy = (float)GetRandomValue(20, 40);
            if (GetRandomValue(0, 1) == 0) dx = -dx;
            if (GetRandomValue(0, 1) == 0) dy = -dy;
        }

        Rectangle startRect = { x1,      y1,      w, h };
        Rectangle endRect   = { x1 + dx, y1 + dy, w, h };

        float minX = fminf(startRect.x, endRect.x) - 16.0f;
        float minY = fminf(startRect.y, endRect.y) - 16.0f;
        float maxX = fmaxf(startRect.x + w, endRect.x + w) + 16.0f;
        float maxY = fmaxf(startRect.y + h, endRect.y + h) + 16.0f;
        Rectangle swept = { minX, minY, maxX - minX, maxY - minY };

        if (!MovingPlatformPlacementSafe(game, L, swept)) continue;

        float speed = 0.25f + (float)GetRandomValue(0, 15) / 100.0f;
        if (MovingPlatform_Add(game->movingPlatforms, startRect, endRect, speed))
            placed++;
    }
}

/* ============================================================
 *  Save / Load progress
 * ============================================================ */

static void SaveProgress(const Game *game) {
    unsigned char buf[MAX_TOTAL_LEVELS * 2];
    for (int i = 0; i < MAX_TOTAL_LEVELS; i++) {
        buf[i]                    = game->levelCompleted[i] ? 1 : 0;
        buf[i + MAX_TOTAL_LEVELS] = game->levelStars[i];
    }
    if (!SaveFileData("save.dat", buf, sizeof(buf))) {
        TraceLog(LOG_WARNING, "Could not write save.dat");
    }
}

static void LoadProgress(Game *game) {
    int size = 0;
    unsigned char *data = LoadFileData("save.dat", &size);

    if (!data) {
        TraceLog(LOG_INFO, "No save file found - starting fresh.");
        return;
    }

    if (size == MAX_TOTAL_LEVELS) {
        for (int i = 0; i < MAX_TOTAL_LEVELS; i++) {
            game->levelCompleted[i] = (data[i] != 0);
            game->levelStars[i]     = data[i] ? 1 : 0;
        }
        TraceLog(LOG_INFO, "Progress loaded (legacy format).");
    } else if (size == MAX_TOTAL_LEVELS * 2) {
        for (int i = 0; i < MAX_TOTAL_LEVELS; i++) {
            game->levelCompleted[i] = (data[i] != 0);
            game->levelStars[i]     = data[i + MAX_TOTAL_LEVELS];
            if (game->levelStars[i] > MAX_STARS)
                game->levelStars[i] = MAX_STARS;
        }
        TraceLog(LOG_INFO, "Progress loaded.");
    } else {
        TraceLog(LOG_WARNING,
                 "Save size mismatch (%d, expected %d or %d). Ignored.",
                 size, MAX_TOTAL_LEVELS, MAX_TOTAL_LEVELS * 2);
    }
    UnloadFileData(data);
}

/* ============================================================
 *  Background
 * ============================================================ */

static void LoadBackground(Game *game) {
    game->background = LoadTexture("assets/background.png");

    if (game->background.id == 0) {
        TraceLog(LOG_ERROR, "FAILED to load assets/background.png");
        Image img = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, MAGENTA);
        game->background = LoadTextureFromImage(img);
        UnloadImage(img);
    } else {
        SetTextureFilter(game->background, TEXTURE_FILTER_POINT);
    }
}

static void DrawBackgroundImage(const Game *game) {
    Rectangle src = { 0, 0,
                      (float)game->background.width,
                      (float)game->background.height };
    Rectangle dst = { 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };

    DrawTexturePro(game->background, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

static void DrawAnimatedOverlay(void) {
    float t = (float)GetTime();

    for (int i = 0; i < 90; i++) {
        int   x     = (i * 137) % SCREEN_WIDTH;
        int   y     = (i * 79)  % (SCREEN_HEIGHT / 2);
        float phase = (float)(i * 13);
        float tw    = 0.5f + 0.5f * sinf(t * 2.0f + phase);

        int alpha = 80 + (int)(tw * 175);
        float r   = 0.8f + (i % 3) * 0.5f;

        DrawCircle(x, y, r, (Color){ 255, 255, 255, (unsigned char)alpha });
    }

    for (int i = 0; i < 40; i++) {
        float sx = (float)((i * 97)  % SCREEN_WIDTH);
        float sy = fmodf((float)(i * 53) + t * 18.0f, (float)SCREEN_HEIGHT);
        DrawCircle(sx, sy, 1.2f, (Color){ 255, 255, 255, 100 });
    }

    for (int i = 0; i < 25; i++) {
        float sx = (float)((i * 173) % SCREEN_WIDTH);
        float sy = fmodf((float)(i * 91) + t * 42.0f, (float)SCREEN_HEIGHT);
        DrawCircle(sx, sy, 2.2f, (Color){ 255, 255, 255, 190 });
    }

    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, 90,
                           (Color){ 0, 0, 0, 80 }, (Color){ 0, 0, 0, 0 });
    DrawRectangleGradientV(0, SCREEN_HEIGHT - 120, SCREEN_WIDTH, 120,
                           (Color){ 0, 0, 0, 0 }, (Color){ 0, 0, 0, 120 });
}

/* ============================================================
 *  Stars
 * ============================================================ */

static void DrawStarShape(float cx, float cy, float outerR,
                          Color col, bool filled)
{
    Vector2 outer[5];
    Vector2 inner[5];

    for (int i = 0; i < 5; i++) {
        float angO = -PI / 2.0f + i * (2.0f * PI / 5.0f);
        float angI = angO + PI / 5.0f;
        outer[i] = (Vector2){ cx + cosf(angO) * outerR,
                              cy + sinf(angO) * outerR };
        inner[i] = (Vector2){ cx + cosf(angI) * outerR * 0.42f,
                              cy + sinf(angI) * outerR * 0.42f };
    }

    if (filled) {
        for (int i = 0; i < 5; i++) {
            int n = (i + 1) % 5;
            DrawTriangle((Vector2){ cx, cy }, outer[i], outer[n], col);
            DrawTriangle(outer[i], inner[i], outer[n], col);
            DrawTriangle(inner[i], (Vector2){ cx, cy }, outer[n], col);
        }
    } else {
        for (int i = 0; i < 5; i++) {
            int n = (i + 1) % 5;
            DrawLineEx(outer[i], inner[i], 2.0f, col);
            DrawLineEx(inner[i], outer[n], 2.0f, col);
        }
    }
}

/* ============================================================
 *  HUD
 * ============================================================ */

static void DrawLivesRow(int lives, int xStart, int y, Color full) {
    for (int i = 0; i < JACK_MAX_LIVES; i++) {
        int cx = xStart + i * 26;
        Color face = (i < lives) ? full : (Color){ 60, 40, 40, 255 };
        DrawCircle(cx, y, 9, face);
        DrawCircleLines(cx, y, 9, (Color){ 20, 15, 15, 255 });
    }
}

static void DrawHUD(const Game *game) {
    DrawText(TextFormat("SCORE: %d", game->score),
             20, 20, 26, (Color){ 232, 245, 255, 255 });

    DrawText(TextFormat("LEVEL %d / %d",
                        game->currentLevel + 1, MAX_TOTAL_LEVELS),
             20, 52, 20, (Color){ 200, 220, 240, 220 });

    DrawLivesRow(game->players[0].lives, 30, 92,
                 (Color){ 120, 200, 255, 255 });

    int p2StartX = SCREEN_WIDTH - 30 - (JACK_MAX_LIVES - 1) * 26;
    DrawLivesRow(game->players[1].lives, p2StartX, 92,
                 (Color){ 255, 110, 110, 255 });

    /* ---- Timer ---- */
    float remaining = game->levelTimer;
    if (remaining < 0.0f) remaining = 0.0f;

    int secs = (int)ceilf(remaining);
    const char *timeStr = TextFormat("%d", secs);
    int  tf = 54;
    int  tw = MeasureText(timeStr, tf);
    int  tx = SCREEN_WIDTH / 2 - tw / 2;
    int  ty = 14;

    Color timeCol;
    if      (secs <= 5)  timeCol = (Color){ 255,  90,  90, 255 };
    else if (secs <= 15) timeCol = (Color){ 255, 180,  90, 255 };
    else                 timeCol = (Color){ 232, 245, 255, 255 };

    DrawText("TIME", SCREEN_WIDTH / 2 - MeasureText("TIME", 16) / 2,
             2, 16, (Color){ 200, 220, 240, 220 });
    DrawText(timeStr, tx + 2, ty + 2, tf, (Color){ 0, 0, 0, 120 });
    DrawText(timeStr, tx,     ty,     tf, timeCol);

    /* ---- Team stars ---- */
    {
        float spacing = 34.0f;
        float startX  = SCREEN_WIDTH * 0.5f - spacing;
        float cy      = 92.0f;

        for (int i = 0; i < MAX_STARS; i++) {
            bool  alive = (i < game->starsRemaining);
            float pop   = 1.0f;

            if (game->starPopTimer > 0.0f && alive) {
                float u = game->starPopTimer / 0.6f;
                pop = 1.0f + 0.35f * u;
            }

            Color col  = alive ? (Color){ 255, 220,  80, 255 }
                               : (Color){  60,  70,  90, 200 };
            Color glow = alive ? (Color){ 255, 240, 160, 100 }
                               : (Color){ 0, 0, 0, 0 };

            float cx = startX + i * spacing;
            if (alive) DrawCircle((int)cx, (int)cy, 22.0f * pop, glow);
            DrawStarShape(cx, cy, 16.0f * pop, col, alive);
            DrawStarShape(cx, cy, 16.0f * pop,
                          (Color){ 30, 30, 55, 255 }, false);
        }
    }

    /* ---- Freeze progress (with counts) ---- */
    {
        int total = 0, done = 0;
        for (int r = 0; r < FROZEN_ROWS; r++) {
            for (int c = 0; c < FROZEN_COLS; c++) {
                if (!game->freezableMask[r][c]) continue;
                total++;
                if (Frozen_IsCellFrozen(&game->frozen, c, r)) done++;
            }
        }
        int pct = (total > 0) ? (done * 100) / total : 100;
        const char *prog = TextFormat("FROZEN: %d%%  (%d / %d)",
                                      pct, done, total);
        DrawText(prog,
                 SCREEN_WIDTH / 2 - MeasureText(prog, 18) / 2,
                 130, 18, (Color){ 200, 230, 255, 220 });
    }

    DrawText("P1: A/D  SPACE  E",
             20, SCREEN_HEIGHT - 26, 16, (Color){ 200, 220, 240, 200 });
    DrawText("P2: ARROWS  UP  M",
             SCREEN_WIDTH - 190, SCREEN_HEIGHT - 26, 16,
             (Color){ 240, 200, 220, 200 });

    if (game->levelCompleteTimer > 0.0f) {
        const char *msg = "LEVEL COMPLETE!";
        int fs = 48;
        int msgY = SCREEN_HEIGHT / 2 - 120;
        DrawText(msg, SCREEN_WIDTH / 2 - MeasureText(msg, fs) / 2,
                 msgY, fs, (Color){ 240, 250, 255, 240 });

        float t = game->levelCompleteAnim;

        Rectangle panel = {
            SCREEN_WIDTH / 2 - 200.0f,
            SCREEN_HEIGHT / 2 - 60.0f,
            400.0f, 140.0f
        };
        DrawRectangleRounded(panel, 0.12f, 8,
                             (Color){ 10, 20, 35, 170 });
        DrawRectangleRoundedLines(panel, 0.12f, 8,
                                  (Color){ 160, 200, 240, 200 });

        float baseY   = SCREEN_HEIGHT / 2 + 8.0f;
        float spacing = 90.0f;
        float startX  = SCREEN_WIDTH / 2 - spacing;

        for (int i = 0; i < MAX_STARS; i++) {
            float local = t - i * 0.35f;
            if (local < 0.0f) continue;

            float pop = 1.0f;
            if (local < 0.25f) {
                float u = local / 0.25f;
                pop = 1.0f + 0.6f * sinf(u * PI);
            }

            bool earned = (i < game->starsAwarded);

            Color col  = earned ? (Color){ 255, 220,  80, 255 }
                                : (Color){  90, 100, 120, 200 };
            Color glow = earned ? (Color){ 255, 240, 160, 110 }
                                : (Color){ 0, 0, 0, 0 };

            float cx = startX + i * spacing;
            float cy = baseY;

            if (earned) DrawCircle((int)cx, (int)cy, 32.0f * pop, glow);

            DrawStarShape(cx, cy, 28.0f * pop, col, earned);
            DrawStarShape(cx, cy, 28.0f * pop,
                          (Color){ 30, 30, 55, 255 }, false);
        }
    }

    if (!game->active && game->levelCompleteTimer <= 0.0f) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                      (Color){ 0, 0, 0, 150 });

        const char *over    = "LEVEL FAILED";
        const char *restart = "Press R to Retry";
        const char *menu    = "Press M for Menu";

        DrawText(over,
                 SCREEN_WIDTH / 2 - MeasureText(over, 52) / 2,
                 SCREEN_HEIGHT / 2 - 60, 52,
                 (Color){ 255, 255, 255, 255 });

        DrawText(restart,
                 SCREEN_WIDTH / 2 - MeasureText(restart, 26) / 2,
                 SCREEN_HEIGHT / 2 + 10, 26,
                 (Color){ 200, 220, 240, 255 });

        DrawText(menu,
                 SCREEN_WIDTH / 2 - MeasureText(menu, 26) / 2,
                 SCREEN_HEIGHT / 2 + 46, 26,
                 (Color){ 200, 220, 240, 255 });
    }
}

/* ============================================================
 *  Camera
 * ============================================================ */

#define CAM_PADDING      260.0f
#define CAM_LERP_SPEED     5.0f
#define CAM_MIN_ZOOM       0.15f
#define CAM_MAX_ZOOM       1.0f

static void ComputeCameraDesired(const Game *game,
                                 float *outZoom, Vector2 *outCenter)
{
    Vector2 p1 = game->players[0].position;
    Vector2 p2 = game->players[1].position;

    bool p1out = (game->players[0].state == JACK_STATE_GONE ||
                  game->players[0].state == JACK_STATE_FLUNG);
    bool p2out = (game->players[1].state == JACK_STATE_GONE ||
                  game->players[1].state == JACK_STATE_FLUNG);
    if (p1out) p1 = p2;
    if (p2out) p2 = p1;

    float minX = fminf(p1.x, p2.x);
    float maxX = fmaxf(p1.x, p2.x);
    float minY = fminf(p1.y, p2.y);
    float maxY = fmaxf(p1.y, p2.y);

    float rangeW = (maxX - minX) + CAM_PADDING;
    float rangeH = (maxY - minY) + CAM_PADDING;

    float zoomX = (float)SCREEN_WIDTH  / rangeW;
    float zoomY = (float)SCREEN_HEIGHT / rangeH;
    float z = fminf(zoomX, zoomY);

    if (z > CAM_MAX_ZOOM) z = CAM_MAX_ZOOM;
    if (z < CAM_MIN_ZOOM) z = CAM_MIN_ZOOM;

    Vector2 center = {
        (minX + maxX) * 0.5f,
        (minY + maxY) * 0.5f
    };

    float worldW = (float)game->currentWorldW;
    float worldH = (float)game->currentWorldH;

    float viewW = (float)SCREEN_WIDTH  / z;
    float viewH = (float)SCREEN_HEIGHT / z;
    float halfW = viewW * 0.5f;
    float halfH = viewH * 0.5f;

    if (viewW >= worldW) center.x = worldW * 0.5f;
    else center.x = fmaxf(halfW, fminf(worldW - halfW, center.x));

    if (viewH >= worldH) center.y = worldH * 0.5f;
    else center.y = fmaxf(halfH, fminf(worldH - halfH, center.y));

    *outZoom   = z;
    *outCenter = center;
}

static void SnapCameraToPlayers(Game *game) {
    float z;
    Vector2 c;
    ComputeCameraDesired(game, &z, &c);

    game->camera.zoom     = z;
    game->camera.target   = c;
    game->camera.offset   = (Vector2){ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };
    game->camera.rotation = 0.0f;
}

static void UpdateCameraForLevel(Game *game, float dt) {
    float   desiredZoom;
    Vector2 desiredCenter;
    ComputeCameraDesired(game, &desiredZoom, &desiredCenter);

    float t = 1.0f - expf(-CAM_LERP_SPEED * dt);

    game->camera.zoom     += (desiredZoom        - game->camera.zoom)     * t;
    game->camera.target.x += (desiredCenter.x    - game->camera.target.x) * t;
    game->camera.target.y += (desiredCenter.y    - game->camera.target.y) * t;

    float worldW = (float)game->currentWorldW;
    float worldH = (float)game->currentWorldH;

    float viewW = (float)SCREEN_WIDTH  / game->camera.zoom;
    float viewH = (float)SCREEN_HEIGHT / game->camera.zoom;
    float halfW = viewW * 0.5f;
    float halfH = viewH * 0.5f;

    if (viewW >= worldW) game->camera.target.x = worldW * 0.5f;
    else game->camera.target.x = fmaxf(halfW, fminf(worldW - halfW,
                                                    game->camera.target.x));

    if (viewH >= worldH) game->camera.target.y = worldH * 0.5f;
    else game->camera.target.y = fmaxf(halfH, fminf(worldH - halfH,
                                                    game->camera.target.y));
}

/* ============================================================
 *  Menu
 * ============================================================ */

typedef enum MenuItem {
    MENU_PLAY = 0,
    MENU_LEVEL_SELECT,
    MENU_CONTROLS,
    MENU_QUIT,
    MENU_COUNT
} MenuItem;

static const char *MENU_LABELS[MENU_COUNT] = {
    "PLAY", "LEVEL SELECT", "CONTROLS", "QUIT"
};

#define MENU_ITEM_W       260
#define MENU_ITEM_H       44
#define MENU_ITEM_SPACING 58
#define MENU_FIRST_Y      230

static Rectangle MenuItemRect(int i) {
    return (Rectangle){
        (float)(SCREEN_WIDTH / 2 - MENU_ITEM_W / 2),
        (float)(MENU_FIRST_Y + i * MENU_ITEM_SPACING),
        (float)MENU_ITEM_W,
        (float)MENU_ITEM_H
    };
}

static int FirstUncompletedLevel(const Game *game) {
    for (int i = 0; i < MAX_LEVELS; i++)
        if (!game->levelCompleted[i]) return i;
    return 0;
}

static void UpdateMenu(Game *game) {
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        game->menuSelection = (game->menuSelection + 1) % MENU_COUNT;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        game->menuSelection = (game->menuSelection + MENU_COUNT - 1) % MENU_COUNT;
    }

    Vector2 mouse = GetMousePosition();
    Vector2 delta = GetMouseDelta();
    bool    moved = (delta.x != 0.0f || delta.y != 0.0f);
    bool    hovered = false;

    for (int i = 0; i < MENU_COUNT; i++) {
        if (CheckCollisionPointRec(mouse, MenuItemRect(i))) {
            if (moved) game->menuSelection = i;
            if (game->menuSelection == i) hovered = true;
        }
    }

    bool activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                    (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));

    if (!activate) return;

    switch (game->menuSelection) {
        case MENU_PLAY: {
            int start = FirstUncompletedLevel(game);
            Game_LoadLevel(game, start);
        } break;
        case MENU_LEVEL_SELECT:
            game->state = GAME_STATE_LEVEL_SELECT;
            game->levelSelectSelection = 0;
            break;
        case MENU_CONTROLS:
            game->state = GAME_STATE_CONTROLS;
            break;
        case MENU_QUIT:
            game->quitRequested = true;
            break;
    }
}

static void DrawMenu(const Game *game) {
    float t = (float)GetTime();

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 90 });

    const char *title    = "JACK FROST";
    const char *subtitle = "Freeze them all";
    int   titleSize = 68;
    float bob       = sinf(t * 2.0f) * 4.0f;
    int   titleX    = SCREEN_WIDTH / 2 - MeasureText(title, titleSize) / 2;
    int   titleY    = 70 + (int)bob;

    DrawText(title, titleX + 3, titleY + 3, titleSize, (Color){ 20, 50, 90, 200 });
    DrawText(title, titleX,     titleY,     titleSize, (Color){ 232, 245, 255, 255 });

    DrawText(subtitle,
             SCREEN_WIDTH / 2 - MeasureText(subtitle, 22) / 2,
             titleY + titleSize + 4, 22, (Color){ 170, 210, 240, 255 });

    for (int i = 0; i < MENU_COUNT; i++) {
        Rectangle r        = MenuItemRect(i);
        bool      selected = (game->menuSelection == i);

        Color fill   = selected ? (Color){ 120, 190, 240, 230 }
                                : (Color){ 20, 40, 70, 170 };
        Color border = selected ? (Color){ 255, 255, 255, 255 }
                                : (Color){ 130, 170, 210, 200 };
        Color text   = selected ? (Color){ 10, 30, 60, 255 }
                                : (Color){ 232, 245, 255, 255 };

        DrawRectangleRec(r, fill);
        DrawRectangleLinesEx(r, 2.0f, border);

        int fontSize = 24;
        DrawText(MENU_LABELS[i],
                 (int)(r.x + r.width / 2) - MeasureText(MENU_LABELS[i], fontSize) / 2,
                 (int)(r.y + (r.height - fontSize) / 2),
                 fontSize, text);
    }

    const char *hint = "Arrows / W,S to move  -  Enter to select";
    DrawText(hint,
             SCREEN_WIDTH / 2 - MeasureText(hint, 16) / 2,
             SCREEN_HEIGHT - 28, 16, (Color){ 200, 220, 240, 200 });
}

/* ============================================================
 *  Level select
 * ============================================================ */

#define LS_COLS    5
#define LS_ROWS    5
#define LS_BTN_W   120
#define LS_BTN_H   55
#define LS_GAP_X   10
#define LS_GAP_Y   10
#define LS_TOP     100

static Rectangle LevelButtonRect(int i) {
    int col = i % LS_COLS;
    int row = i / LS_COLS;

    float totalW = LS_COLS * LS_BTN_W + (LS_COLS - 1) * LS_GAP_X;
    float startX = (SCREEN_WIDTH - totalW) / 2.0f;

    return (Rectangle){
        startX + col * (LS_BTN_W + LS_GAP_X),
        (float)LS_TOP + row * (LS_BTN_H + LS_GAP_Y),
        (float)LS_BTN_W,
        (float)LS_BTN_H
    };
}

/* A custom level is only playable if it contains at least one platform. */
static bool CustomLevel_HasContent(const CustomLevel *c) {
    return c->nPlatforms > 0;
}

static bool LevelUnlocked(const Game *game, int idx) {
    if (idx < MAX_LEVELS) {
        if (idx == 0) return true;
        return game->levelCompleted[idx - 1];
    }
    int slot = idx - MAX_LEVELS;
    if (slot < 0) slot = 0;
    if (slot >= MAX_CUSTOM_LEVELS) slot = MAX_CUSTOM_LEVELS - 1;
    return CustomLevel_HasContent(&game->customLevels[slot]);
}

static int FirstEmptyCustomSlot(const Game *game) {
    for (int i = 0; i < MAX_CUSTOM_LEVELS; i++)
        if (!CustomLevel_HasContent(&game->customLevels[i])) return i;
    return 0;
}

static void UpdateLevelSelect(Game *game) {
    int col = game->levelSelectSelection % LS_COLS;
    int row = game->levelSelectSelection / LS_COLS;

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        col = (col + 1) % LS_COLS;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        col = (col + LS_COLS - 1) % LS_COLS;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        row = (row + 1) % LS_ROWS;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        row = (row + LS_ROWS - 1) % LS_ROWS;

    game->levelSelectSelection = row * LS_COLS + col;

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
        game->state = GAME_STATE_MENU;
        return;
    }

    if (IsKeyPressed(KEY_E)) {
        int idx = game->levelSelectSelection;
        if (idx >= MAX_LEVELS) {
            Editor_Enter(&game->editor, idx - MAX_LEVELS);
            game->state = GAME_STATE_EDITOR;
        } else {
            Editor_Enter(&game->editor, FirstEmptyCustomSlot(game));
            game->state = GAME_STATE_EDITOR;
        }
        return;
    }

    Vector2 mouse = GetMousePosition();
    Vector2 delta = GetMouseDelta();
    bool    moved = (delta.x != 0.0f || delta.y != 0.0f);
    bool    hovered = false;

    for (int i = 0; i < MAX_TOTAL_LEVELS; i++) {
        if (CheckCollisionPointRec(mouse, LevelButtonRect(i))) {
            if (moved) game->levelSelectSelection = i;
            if (game->levelSelectSelection == i) hovered = true;
        }
    }

    bool activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                    (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));

    if (!activate) return;

    int idx = game->levelSelectSelection;

    if (idx >= MAX_LEVELS) {
        int slot = idx - MAX_LEVELS;
        if (!CustomLevel_HasContent(&game->customLevels[slot])) {
            Editor_Enter(&game->editor, slot);
            game->state = GAME_STATE_EDITOR;
            return;
        }
    }

    if (LevelUnlocked(game, idx)) {
        Game_LoadLevel(game, idx);
    }
}

static void DrawLevelSelect(const Game *game) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 150 });

    const char *title = "LEVEL SELECT";
    DrawText(title,
             SCREEN_WIDTH / 2 - MeasureText(title, 42) / 2,
             35, 42, (Color){ 232, 245, 255, 255 });

    for (int i = 0; i < MAX_TOTAL_LEVELS; i++) {
        Rectangle r          = LevelButtonRect(i);
        bool      isCustom   = (i >= MAX_LEVELS);
        bool      hasContent = isCustom
                             ? CustomLevel_HasContent(&game->customLevels[i - MAX_LEVELS])
                             : true;
        bool      unlocked   = LevelUnlocked(game, i);
        bool      done       = game->levelCompleted[i];
        bool      selected   = (game->levelSelectSelection == i);

        Color fill, border, text;
        if (isCustom && !hasContent) {
            fill   = (Color){  20,  30,  40, 200 };
            border = (Color){  90, 130, 170, 220 };
            text   = (Color){ 150, 190, 220, 255 };
        } else if (!unlocked) {
            fill   = (Color){  20,  20,  30, 200 };
            border = (Color){  70,  70,  90, 220 };
            text   = (Color){ 120, 130, 150, 255 };
        } else if (done) {
            fill   = (Color){  60, 180, 120, 220 };
            border = (Color){ 200, 255, 220, 255 };
            text   = (Color){  15,  40,  25, 255 };
        } else {
            fill   = (Color){  40,  90, 140, 220 };
            border = (Color){ 150, 210, 250, 255 };
            text   = (Color){ 232, 245, 255, 255 };
        }

        if (selected) {
            fill   = (Color){ (unsigned char)(fill.r + 40),
                              (unsigned char)(fill.g + 40),
                              (unsigned char)(fill.b + 40),
                              fill.a };
            border = (Color){ 255, 255, 255, 255 };
        }

        DrawRectangleRec(r, fill);
        DrawRectangleLinesEx(r, 2.5f, border);

        const char *num = TextFormat("%d", i + 1);
        int  fs = 22;
        DrawText(num,
                 (int)(r.x + r.width / 2) - MeasureText(num, fs) / 2,
                 (int)(r.y + 2),
                 fs, text);

        if (done && !isCustom) {
            int stars = game->levelStars[i];
            if (stars > MAX_STARS) stars = MAX_STARS;

            float spacing = 16.0f;
            float cy      = r.y + r.height - 22.0f;
            float cx      = r.x + r.width * 0.5f - spacing;

            for (int s = 0; s < MAX_STARS; s++) {
                bool earned = (s < stars);
                Color col = earned ? (Color){ 255, 220,  80, 255 }
                                   : (Color){  60,  70,  90, 200 };
                float sx = cx + s * spacing;
                float sy = cy;
                DrawStarShape(sx, sy, 7.0f, col, earned);
                DrawStarShape(sx, sy, 7.0f,
                              (Color){ 20, 25, 40, 255 }, false);
            }
        }

        const char *name;
        int  ns = 11;
        if (isCustom && !hasContent) {
            name = "CREATE";
        } else if (isCustom) {
            name = game->customLevels[i - MAX_LEVELS].name;
        } else {
            name = LEVELS[i].name;
        }
        if (!unlocked && !isCustom) name = "LOCKED";

        int nameY = (int)(r.y + r.height - 12);
        DrawText(name,
                 (int)(r.x + r.width / 2) - MeasureText(name, ns) / 2,
                 nameY,
                 ns, text);
    }

    const char *hint = "Arrows move  -  Enter play  -  E edit/create  -  Esc back";
    DrawText(hint,
             SCREEN_WIDTH / 2 - MeasureText(hint, 15) / 2,
             SCREEN_HEIGHT - 22, 15, (Color){ 200, 220, 240, 200 });
}

/* ============================================================
 *  Controls screen
 * ============================================================ */

static void UpdateControls(Game *game) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_SPACE)  || IsKeyPressed(KEY_BACKSPACE) ||
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game->state = GAME_STATE_MENU;
    }
}

static void DrawControls(void) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 150 });

    const char *title = "CONTROLS";
    DrawText(title,
             SCREEN_WIDTH / 2 - MeasureText(title, 48) / 2,
             70, 48, (Color){ 232, 245, 255, 255 });

    const char *lines[] = {
        "Player 1:  A / D to move,  Space to jump,  E to shoot",
        "Player 2:  Left / Right to move,  Up to jump,  M to shoot",
        "Stun enemies with shards - don't touch them!",
        "Freeze every tile you can reach to complete the level.",
        "You have 60 seconds. You lose a star every 20 seconds.",
        "Running out of time fails the level.",
        "",
        "Level Editor:  number keys pick tools, Q/R cycle enemy type"
    };
    int count = (int)(sizeof(lines) / sizeof(lines[0]));

    for (int i = 0; i < count; i++) {
        DrawText(lines[i],
                 SCREEN_WIDTH / 2 - MeasureText(lines[i], 18) / 2,
                 160 + i * 32, 18, (Color){ 200, 225, 245, 255 });
    }

    const char *back = "Press Esc or click to go back";
    DrawText(back,
             SCREEN_WIDTH / 2 - MeasureText(back, 18) / 2,
             SCREEN_HEIGHT - 50, 18, (Color){ 170, 210, 240, 220 });
}

/* ============================================================
 *  Level loading / spawning
 * ============================================================ */

/* Mark grid cells the player can actually touch.
 * A cell counts only if its CENTER lies inside an active platform.
 * No band above platforms, no phantom edge tiles, no ground fallback. */
static void MarkFreezableTiles(Game *game) {
    memset(game->freezableMask, 0, sizeof(game->freezableMask));

    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!game->platforms[i].active) continue;

        Rectangle p = game->platforms[i].bounds;

        /* Optional forgiveness: inflate by half a tile so the tiles the
         * player stands on the very edge of also count. Remove the four
         * lines below if you want STRICT platform-only cells. */
        p.x      -= FROZEN_TILE_SIZE * 0.25f;
        p.y      -= FROZEN_TILE_SIZE * 0.25f;
        p.width  += FROZEN_TILE_SIZE * 0.5f;
        p.height += FROZEN_TILE_SIZE * 0.5f;

        /* Only cells whose CENTER lies inside the platform */
        int c0 = (int)ceilf ((p.x - FROZEN_TILE_SIZE * 0.5f) / FROZEN_TILE_SIZE);
        int c1 = (int)floorf((p.x + p.width  - FROZEN_TILE_SIZE * 0.5f) / FROZEN_TILE_SIZE);
        int r0 = (int)ceilf ((p.y - FROZEN_TILE_SIZE * 0.5f) / FROZEN_TILE_SIZE);
        int r1 = (int)floorf((p.y + p.height - FROZEN_TILE_SIZE * 0.5f) / FROZEN_TILE_SIZE);

        if (c0 < 0) c0 = 0;
        if (r0 < 0) r0 = 0;
        if (c1 >= FROZEN_COLS) c1 = FROZEN_COLS - 1;
        if (r1 >= FROZEN_ROWS) r1 = FROZEN_ROWS - 1;

        for (int r = r0; r <= r1; r++) {
            for (int c = c0; c <= c1; c++) {
                game->freezableMask[r][c] = 1;
            }
        }
    }
}

static void LoadLevelPlatforms(Game *game, const LevelDef *L) {
    Platform_InitAll(game->platforms);
    for (int i = 0; i < L->nPlatforms && i < MAX_PLATFORMS; i++) {
        Platform_Add(game->platforms, L->platforms[i]);
    }

    BuildLadders(game);
}

/* ============================================================
 *  Ladder generation
 * ============================================================ */

static bool LadderBlockedByPlatform(const Game *game,
                                    float x0, float x1,
                                    float topY, float bottomY) {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!game->platforms[i].active) continue;

        Rectangle p = game->platforms[i].bounds;

        bool hOverlap = (p.x < x1) && (p.x + p.width > x0);
        if (!hOverlap) continue;

        if (p.y > topY + 1.0f && p.y < bottomY - 1.0f) {
            return true;
        }
    }
    return false;
}

static float NextPlatformTopBelow(const Game *game,
                                  float x0, float x1, float topY) {
    float best = 1e9f;
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!game->platforms[i].active) continue;
        Rectangle p = game->platforms[i].bounds;

        if (p.y <= topY + 1.0f) continue;

        bool hOverlap = (p.x < x1) && (p.x + p.width > x0);
        if (!hOverlap) continue;

        if (p.y < best) best = p.y;
    }
    return best;
}

static void BuildLadders(Game *game) {
    Ladder_InitAll(game->ladders);

    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!game->platforms[i].active) continue;

        Rectangle upper = game->platforms[i].bounds;

        if (upper.y >= 440.0f) continue;

        float ladderX = upper.x + upper.width * 0.5f - LADDER_WIDTH * 0.5f;
        if (ladderX < upper.x)
            ladderX = upper.x;
        if (ladderX + LADDER_WIDTH > upper.x + upper.width)
            ladderX = upper.x + upper.width - LADDER_WIDTH;

        float ladderTop = upper.y - 4.0f;

        float lowerTop = NextPlatformTopBelow(game, ladderX,
                                              ladderX + LADDER_WIDTH,
                                              upper.y + upper.height);

        if (lowerTop > 1e8f) continue;

        float ladderBottom = lowerTop - 2.0f;

        if (ladderBottom <= ladderTop + 8.0f) continue;

        if (LadderBlockedByPlatform(game, ladderX, ladderX + LADDER_WIDTH,
                                    ladderTop + 4.0f, ladderBottom)) {
            continue;
        }

        Ladder_Add(game->ladders,
                   (Rectangle){ ladderX, ladderTop,
                                LADDER_WIDTH, ladderBottom - ladderTop });
    }
}

/* After all ladders are added, merge any that are stacked end-to-end
 * into a single logical ladder. This makes chains of ladders behave
 * as one continuous ladder with a single standable top. */
static void MergeStackedLadders(Ladder ladders[MAX_LADDERS]) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < MAX_LADDERS; i++) {
            if (!ladders[i].active) continue;

            for (int j = i + 1; j < MAX_LADDERS; j++) {
                if (!ladders[j].active) continue;

                Rectangle a = ladders[i].bounds;
                Rectangle b = ladders[j].bounds;

                /* X overlap with a small tolerance for staggered ladders */
                if (a.x > b.x + b.width + 6.0f) continue;
                if (b.x > a.x + a.width + 6.0f) continue;

                float aBottom = a.y + a.height;
                float bBottom = b.y + b.height;

                /* b directly below a (gap <= 4 px) → extend a downward */
                if (fabsf(aBottom - b.y) <= 4.0f) {
                    ladders[i].bounds.height = bBottom - a.y;
                    ladders[j].active        = false;
                    changed                  = true;
                }
                /* b directly above a (gap <= 4 px) → extend a upward */
                else if (fabsf(bBottom - a.y) <= 4.0f) {
                    ladders[i].bounds.y      = b.y;
                    ladders[i].bounds.height = aBottom - b.y;
                    ladders[j].active        = false;
                    changed                  = true;
                }
            }
        }
    }
}

/* ============================================================
 *  Global Y offset
 *
 *  Applied once, right after a level finishes loading, to
 *  nudge every world element down by a fixed amount. Keeps
 *  everything aligned with each other while letting the
 *  visuals sit a few pixels lower than the raw level data.
 * ============================================================ */

static void ApplyLevelYShift(Game *game) {
    for (int i = 0; i < MAX_PLATFORMS; i++)
        if (game->platforms[i].active)
            game->platforms[i].bounds.y += LEVEL_Y_SHIFT;

    for (int i = 0; i < MAX_LADDERS; i++)
        if (game->ladders[i].active)
            game->ladders[i].bounds.y += LEVEL_Y_SHIFT;

    for (int i = 0; i < MAX_MOVING_PLATFORMS; i++) {
        if (!game->movingPlatforms[i].active) continue;
        game->movingPlatforms[i].bounds.y   += LEVEL_Y_SHIFT;
        game->movingPlatforms[i].startPos.y += LEVEL_Y_SHIFT;
        game->movingPlatforms[i].endPos.y   += LEVEL_Y_SHIFT;
        game->movingPlatforms[i].prevPos.y  += LEVEL_Y_SHIFT;
    }

    for (int i = 0; i < MAX_JUMPPADS; i++)
        if (game->jumppads[i].active)
            game->jumppads[i].bounds.y += LEVEL_Y_SHIFT;

    for (int i = 0; i < MAX_SPIKES; i++)
        if (game->spikes[i].active)
            game->spikes[i].bounds.y += LEVEL_Y_SHIFT;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!game->enemies[i].active) continue;
        game->enemies[i].position.y += LEVEL_Y_SHIFT;
        game->enemies[i].baseY      += LEVEL_Y_SHIFT;
    }

    for (int i = 0; i < PLAYER_COUNT; i++) {
        game->players[i].position.y      += LEVEL_Y_SHIFT;
        game->players[i].spawnPosition.y += LEVEL_Y_SHIFT;
    }
}

static void SpawnEnemiesForLevel(Game *game, const LevelDef *L) {
    for (int e = 0; e < L->nEnemies && e < MAX_ENEMIES; e++) {
        int idx = L->enemies[e].platformIdx;
        if (idx < 0 || idx >= L->nPlatforms) continue;

        Rectangle plat = L->platforms[idx];

        switch (L->enemies[e].type) {
            case ENEMY_SHOOTER:
                Enemy_SpawnShooterOnPlatform(game->enemies, plat);
                break;
            case ENEMY_DASHER:
                Enemy_SpawnDasherOnPlatform(game->enemies, plat);
                break;
            case ENEMY_FLYER:
                Enemy_SpawnFlyer(game->enemies, plat);
                break;
            case ENEMY_UNSTUNNABLE:
                Enemy_SpawnUnstunnableOnPlatform(game->enemies, plat);
                break;
            case ENEMY_WALKER:
            default:
                Enemy_SpawnOnPlatform(game->enemies, plat);
                break;
        }
    }
}

static void CompleteLevel(Game *game) {
    int idx = game->currentLevel;
    if (idx < 0) idx = 0;
    if (idx >= MAX_TOTAL_LEVELS) idx = MAX_TOTAL_LEVELS - 1;

    int s = game->starsRemaining;
    if (s < 0) s = 0;
    if (s > MAX_STARS) s = MAX_STARS;

    if (!game->levelCompleted[idx]) {
        game->levelCompleted[idx] = true;
        game->levelStars[idx]     = (unsigned char)s;
    } else if (s > game->levelStars[idx]) {
        game->levelStars[idx] = (unsigned char)s;
    }

    SaveProgress(game);

    game->starsAwarded       = game->levelStars[idx];
    game->levelCompleteAnim  = 0.0f;
    game->levelCompleteTimer = 2.5f;
}

void Game_LoadLevel(Game *game, int idx) {
    if (idx < 0)                 idx = 0;
    if (idx >= MAX_TOTAL_LEVELS) idx = MAX_TOTAL_LEVELS - 1;

    game->currentLevel = idx;

    Platform_InitAll(game->platforms);
    Shard_InitAll(game->shards);
    Enemy_InitAll(game->enemies);
    EnemyProjectile_InitAll(game->enemyProjectiles);
    JumpPad_InitAll(game->jumppads);
    Spike_InitAll(game->spikes);
    MovingPlatform_InitAll(game->movingPlatforms);
    Ladder_InitAll(game->ladders);
    Frozen_Init(&game->frozen);
    Particle_InitAll(&game->particles);

    if (idx < MAX_LEVELS) {
        const LevelDef *L = &LEVELS[idx];

        for (int i = 0; i < L->nPlatforms && i < MAX_PLATFORMS; i++)
            Platform_Add(game->platforms, L->platforms[i]);
        BuildLadders(game);
        MergeStackedLadders(game->ladders);

        Jack_Init(&game->players[0], L->spawn1, WHITE);
        Jack_Init(&game->players[1], L->spawn2,
                  (Color){ 255, 110, 110, 255 });

        SpawnEnemiesForLevel(game, L);
        PlaceJumppadsRandomly(game, L);
        PlaceSpikesRandomly(game, L);

        int movingTarget = 1 + (idx / 8);
        if (movingTarget > 3) movingTarget = 3;
        PlaceMovingPlatformsRandomly(game, L, movingTarget);

        game->currentWorldW = L->worldW;
        game->currentWorldH = L->worldH;

    } else {
        int slot = idx - MAX_LEVELS;
        if (slot < 0) slot = 0;
        if (slot >= MAX_CUSTOM_LEVELS) slot = MAX_CUSTOM_LEVELS - 1;

        CustomLevel *C = &game->customLevels[slot];

        for (int i = 0; i < C->nPlatforms && i < MAX_PLATFORMS; i++)
            Platform_Add(game->platforms, C->platforms[i]);

        for (int i = 0; i < C->nLadders && i < MAX_LADDERS; i++)
            Ladder_Add(game->ladders, C->ladders[i]);
        MergeStackedLadders(game->ladders);

        for (int i = 0; i < C->nSpikes && i < MAX_SPIKES; i++)
            Spike_Add(game->spikes, C->spikes[i]);

        for (int i = 0; i < C->nJumppads && i < MAX_JUMPPADS; i++)
            JumpPad_Add(game->jumppads, C->jumppads[i]);

        for (int i = 0; i < C->nMoving && i < MAX_MOVING_PLATFORMS; i++) {
            MovingPlatform_Add(game->movingPlatforms,
                               C->moving[i].startRect,
                               C->moving[i].endRect,
                               0.35f);
        }

        for (int i = 0; i < C->nEnemies && i < MAX_ENEMIES; i++) {
            EditorEnemy *se = &C->enemies[i];
            Enemy_PlaceDirect(game->enemies, se->position, se->baseY,
                              se->patrolLeft, se->patrolRight,
                              (EnemyType)se->type);
        }

        Jack_Init(&game->players[0], C->spawn1, WHITE);
        Jack_Init(&game->players[1], C->spawn2,
                  (Color){ 255, 110, 110, 255 });

        game->currentWorldW = EDITOR_WORLD_W;
        game->currentWorldH = EDITOR_WORLD_H;
    }


    ApplyLevelYShift(game);

    game->active             = true;
    game->state              = GAME_STATE_PLAYING;
    game->frameCounter       = 0.0f;
    game->levelCompleteTimer = 0.0f;
    game->levelTimer         = 5.0f;
    game->levelCompleteAnim   = 0.0f;
    game->starsAwarded        = 0;


    game->levelTimer     = 60.0f;
    game->starsRemaining = MAX_STARS;
    game->starMilestone1 = 40.0f;
    game->starMilestone2 = 20.0f;
    game->starLost1      = false;
    game->starLost2      = false;
    game->starPopTimer   = 0.0f;

    MarkFreezableTiles(game);
    SnapCameraToPlayers(game);
}

/* ============================================================
 *  Collision helpers
 * ============================================================ */

static void ResolveShardPlatformCollisions(Game *game) {
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (!game->shards[i].active) continue;

        Rectangle sr = {
            game->shards[i].position.x - SHARD_RADIUS,
            game->shards[i].position.y - SHARD_RADIUS,
            SHARD_RADIUS * 2,
            SHARD_RADIUS * 2
        };

        for (int j = 0; j < MAX_PLATFORMS; j++) {
            if (!game->platforms[j].active) continue;
            if (CheckCollisionRecs(sr, game->platforms[j].bounds)) {
                game->shards[i].active = false;
                break;
            }
        }
    }
}

static void ResolveShardEnemyCollisions(Game *game) {
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (!game->shards[i].active) continue;

        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!game->enemies[j].active) continue;

            Rectangle er = Enemy_GetRect(&game->enemies[j]);
            if (CheckCollisionCircleRec(game->shards[i].position, SHARD_RADIUS, er)) {
                game->shards[i].active = false;
                if (Enemy_Stun(&game->enemies[j])) {
                    game->score += 10;
                }
                break;
            }
        }
    }
}

static void ResolveShardProjectileCollisions(Game *game) {
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (!game->shards[i].active) continue;

        for (int j = 0; j < MAX_ENEMY_PROJECTILES; j++) {
            if (!game->enemyProjectiles[j].active) continue;

            float dx = game->shards[i].position.x - game->enemyProjectiles[j].position.x;
            float dy = game->shards[i].position.y - game->enemyProjectiles[j].position.y;
            float rr = SHARD_RADIUS + ENEMY_PROJECTILE_RADIUS;

            if (dx * dx + dy * dy < rr * rr) {
                game->shards[i].active = false;
                game->enemyProjectiles[j].active = false;
                break;
            }
        }
    }
}

static void ApplyPlayerThreatCollisions(Game *game,
                                        const Rectangle playerHitboxes[PLAYER_COUNT])
{
    for (int p = 0; p < PLAYER_COUNT; p++) {
        if (!Jack_IsVulnerable(&game->players[p])) continue;

        bool hit = false;

        for (int i = 0; i < MAX_ENEMIES && !hit; i++) {
            if (!game->enemies[i].active)           continue;
            if ( game->enemies[i].stunTimer > 0.0f) continue;
            if (CheckCollisionRecs(Enemy_GetRect(&game->enemies[i]),
                                   playerHitboxes[p])) {
                hit = true;
            }
        }

        for (int j = 0; j < MAX_ENEMY_PROJECTILES && !hit; j++) {
            if (!game->enemyProjectiles[j].active) continue;
            if (CheckCollisionCircleRec(game->enemyProjectiles[j].position,
                                        ENEMY_PROJECTILE_RADIUS,
                                        playerHitboxes[p])) {
                game->enemyProjectiles[j].active = false;
                hit = true;
            }
        }

        if (hit) Jack_Hit(&game->players[p]);
    }
}

/* ============================================================
 *  Public API
 * ============================================================ */

static const JackControls P1_CONTROLS = { KEY_A, KEY_D, KEY_W, KEY_S, KEY_SPACE };
static const JackControls P2_CONTROLS = { KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_UP };

void Game_Init(Game *game) {
    memset(game->levelCompleted, 0, sizeof(game->levelCompleted));
    memset(game->levelStars,     0, sizeof(game->levelStars));
    memset(game->freezableMask,  0, sizeof(game->freezableMask));

    for (int i = 0; i < MAX_CUSTOM_LEVELS; i++) {
        Editor_InitCustomLevel(&game->customLevels[i]);
    }
    game->editor.active = false;
    game->editor.slot   = 0;

    game->currentLevel         = 0;
    game->menuSelection        = 0;
    game->levelSelectSelection = 0;
    game->quitRequested        = false;
    game->score                = 0;
    game->active               = false;
    game->levelCompleteTimer   = 0.0f;
    game->levelCompleteAnim    = 0.0f;
    game->starsAwarded         = 0;
    game->levelTimer           = 0.0f;
    game->frameCounter         = 0.0f;
    game->currentWorldW        = 1200;
    game->currentWorldH        = 620;
    game->state                = GAME_STATE_MENU;
    game->camera               = (Camera2D){ 0 };

    game->starsRemaining = MAX_STARS;
    game->starMilestone1 = 40.0f;
    game->starMilestone2 = 20.0f;
    game->starLost1      = false;
    game->starLost2      = false;
    game->starPopTimer   = 0.0f;
}

void Game_Restart(Game *game) {
    Game_LoadLevel(game, game->currentLevel);
}

void Game_Update(Game *game) {
    if (!game->active) return;

    float dt = GetFrameTime();

    if (game->levelCompleteTimer > 0.0f) {
        game->levelCompleteTimer -= dt;
        game->levelCompleteAnim  += dt;

        if (game->levelCompleteTimer <= 0.0f) {
            game->levelCompleteTimer = 0.0f;

            int next = game->currentLevel + 1;

            /* Do not auto-advance into an empty custom level. */
            bool validNext = (next < MAX_TOTAL_LEVELS);
            if (validNext && next >= MAX_LEVELS) {
                int slot = next - MAX_LEVELS;
                if (!CustomLevel_HasContent(&game->customLevels[slot]))
                    validNext = false;
            }

            if (!validNext) {
                game->state         = GAME_STATE_MENU;
                game->menuSelection = 0;
                return;
            }
            Game_LoadLevel(game, next);
            return;
        }
        UpdateCameraForLevel(game, dt);
        return;
    }

    MovingPlatform_UpdateAll(game->movingPlatforms, dt);
    JumpPad_UpdateAll(game->jumppads, dt);

    for (int p = 0; p < PLAYER_COUNT; p++) {
        Jack *j = &game->players[p];
        if (!Jack_IsPlayable(j))    continue;
        if (!j->onGround)           continue;

        float feetY = j->position.y + j->radiusY;

        for (int m = 0; m < MAX_MOVING_PLATFORMS; m++) {
            MovingPlatform *mp = &game->movingPlatforms[m];
            if (!mp->active) continue;

            float platLeft  = mp->prevPos.x;
            float platTop   = mp->prevPos.y;
            float platRight = platLeft + mp->bounds.width;

            if (j->position.x < platLeft)  continue;
            if (j->position.x > platRight) continue;
            if (fabsf(feetY - platTop) > 14.0f) continue;

            j->position.x += mp->bounds.x - mp->prevPos.x;
            j->position.y += mp->bounds.y - mp->prevPos.y;

            if (j->velocityY > 0.0f) j->velocityY = 0.0f;
            break;
        }
    }

    Jack_Update(&game->players[0], game->platforms, game->ladders,
                game->movingPlatforms, &P1_CONTROLS,
                game->currentWorldW, game->currentWorldH);
    Jack_Update(&game->players[1], game->platforms, game->ladders,
                game->movingPlatforms, &P2_CONTROLS,
                game->currentWorldW, game->currentWorldH);

    /* --- 4. Freeze / particles --- */
    float radiusPx = FROZEN_RADIUS_TILES * FROZEN_TILE_SIZE;

    for (int p = 0; p < PLAYER_COUNT; p++) {
        Jack *j = &game->players[p];
        if (j->state != JACK_STATE_ALIVE) continue;

        Vector2 feet = { j->position.x, j->position.y + j->radiusY };
        Frozen_FreezeCircle(&game->frozen, feet, radiusPx);

        bool moving = IsKeyDown(p == 0 ? KEY_A : KEY_LEFT) ||
                      IsKeyDown(p == 0 ? KEY_D : KEY_RIGHT);

        if (moving || j->climbing) {
            Particle_SpawnIcy(&game->particles, feet, 30.0f);
        }
    }


    /* --- Count freezable vs frozen, and win when all freezable tiles are frozen --- */
    int totalFreezable = 0;
    int frozenCount    = 0;
    int firstUnfrozenR = -1;
    int firstUnfrozenC = -1;

    for (int r = 0; r < FROZEN_ROWS; r++) {
        for (int c = 0; c < FROZEN_COLS; c++) {
            if (!game->freezableMask[r][c]) continue;
            totalFreezable++;
            if (Frozen_IsCellFrozen(&game->frozen, c, r)) {
                frozenCount++;
            } else if (firstUnfrozenR < 0) {
                firstUnfrozenR = r;
                firstUnfrozenC = c;
            }
        }
    }

    /* Debug: print once per second */
    static float debugTimer = 0.0f;
    debugTimer += dt;
    if (debugTimer >= 1.0f) {
        debugTimer = 0.0f;
        TraceLog(LOG_INFO,
                 "Freeze progress: %d / %d frozen  (first unfrozen: r=%d c=%d)",
                 frozenCount, totalFreezable,
                 firstUnfrozenR, firstUnfrozenC);
    }

    bool allFrozen = (totalFreezable > 0) &&
                     (frozenCount == totalFreezable);

    if (allFrozen) {
        CompleteLevel(game);
        return;
    }

    Particle_UpdateAll(&game->particles, GetFrameTime());


    Shard_UpdateAll(game->shards, game->currentWorldW);
    ResolveShardPlatformCollisions(game);

    Vector2   playerPositions[PLAYER_COUNT];
    Rectangle playerHitboxes [PLAYER_COUNT];
    for (int p = 0; p < PLAYER_COUNT; p++) {
        playerPositions[p] = game->players[p].position;
        playerHitboxes[p]  = (Rectangle){
            game->players[p].position.x - JACK_RADIUS * 0.75f,
            game->players[p].position.y - JACK_RADIUS * 0.75f,
            JACK_RADIUS * 1.5f,
            JACK_RADIUS * 1.5f
        };
    }

    Rectangle nullHitboxes[PLAYER_COUNT];
    for (int p = 0; p < PLAYER_COUNT; p++)
        nullHitboxes[p] = (Rectangle){ -10000, -10000, 0, 0 };

    Enemy_UpdateAll(game->enemies, playerPositions, nullHitboxes,
                    PLAYER_COUNT, game->enemyProjectiles);

    EnemyProjectile_UpdateAll(game->enemyProjectiles, nullHitboxes,
                              PLAYER_COUNT,
                              game->currentWorldW, game->currentWorldH);

    ApplyPlayerThreatCollisions(game, playerHitboxes);

    for (int p = 0; p < PLAYER_COUNT; p++) {
        Jack *j = &game->players[p];
        if (!Jack_IsPlayable(j)) continue;

        Rectangle jr = {
            j->position.x - j->radiusX,
            j->position.y - j->radiusY,
            j->radiusX * 2,
            j->radiusY * 2
        };
        if (JumpPad_TryTrigger(game->jumppads, jr)) {
            j->velocityY = -JUMPPAD_LAUNCH_VY;
            j->onGround  = false;
        }
    }

    for (int e = 0; e < MAX_ENEMIES; e++) {
        Enemy *en = &game->enemies[e];
        if (!en->active)             continue;
        if (en->airborne)            continue;
        if (en->stunTimer > 0.0f)    continue;
        if (en->jumpCooldown > 0.0f) continue;

        Rectangle er = Enemy_GetRect(en);
        if (JumpPad_TryTrigger(game->jumppads, er)) {
            en->velocityY = -JUMPPAD_LAUNCH_VY;
            en->airborne  = true;
        }
    }

    for (int p = 0; p < PLAYER_COUNT; p++) {
        Jack *j = &game->players[p];
        if (!Jack_IsVulnerable(j)) continue;

        Rectangle jr = {
            j->position.x - j->radiusX * 0.75f,
            j->position.y - j->radiusY * 0.75f,
            j->radiusX * 1.5f,
            j->radiusY * 1.5f
        };
        if (Spike_OverlapsRect(game->spikes, jr)) {
            Jack_Hit(j);
        }
    }

    if (game->players[0].state == JACK_STATE_GONE &&
        game->players[1].state == JACK_STATE_GONE) {
        game->active = false;
        return;
    }

    ResolveShardEnemyCollisions(game);
    ResolveShardProjectileCollisions(game);

    if (IsKeyPressed(KEY_E) && Jack_IsPlayable(&game->players[0])) {
        Shard_Shoot(game->shards, game->players[0].position,
                    game->players[0].facingDir);
        Jack_TriggerShoot(&game->players[0]);
    }
    if (IsKeyPressed(KEY_M) && Jack_IsPlayable(&game->players[1])) {
        Shard_Shoot(game->shards, game->players[1].position,
                    game->players[1].facingDir);
        Jack_TriggerShoot(&game->players[1]);
    }

    /* --- Timer: 60s, lose a star every 20s, fail at 0 --- */
    game->levelTimer -= dt;

    if (!game->starLost1 && game->levelTimer <= game->starMilestone1) {
        game->starLost1 = true;
        if (game->starsRemaining > 0) game->starsRemaining--;
        game->starPopTimer = 0.6f;
    }
    if (!game->starLost2 && game->levelTimer <= game->starMilestone2) {
        game->starLost2 = true;
        if (game->starsRemaining > 0) game->starsRemaining--;
        game->starPopTimer = 0.6f;
    }

    if (game->starPopTimer > 0.0f) {
        game->starPopTimer -= dt;
        if (game->starPopTimer < 0.0f) game->starPopTimer = 0.0f;
    }

    if (game->levelTimer <= 0.0f) {
        game->levelTimer     = 0.0f;
        game->starsRemaining = 0;
        game->active         = false;
        return;
    }

    UpdateCameraForLevel(game, dt);
}

void Game_Draw(const Game *game) {
    DrawBackgroundImage(game);

    if (game->state == GAME_STATE_EDITOR) {
        Editor_Draw(game);
        return;
    }

    if (game->state == GAME_STATE_PLAYING) {
        BeginMode2D(game->camera);

            Ladder_DrawAll(game->ladders, &game->frozen);
            Platform_DrawAll(game->platforms, &game->frozen);
            MovingPlatform_DrawAll(game->movingPlatforms);
            JumpPad_DrawAll(game->jumppads);
            Spike_DrawAll(game->spikes);
            Shard_DrawAll(game->shards);
            Enemy_DrawAll(game->enemies);
            EnemyProjectile_DrawAll(game->enemyProjectiles);
            Jack_Draw(&game->players[0], &game->sprites);
            Jack_Draw(&game->players[1], &game->sprites);

        EndMode2D();
    }

    DrawAnimatedOverlay();

    switch (game->state) {
        case GAME_STATE_MENU:          DrawMenu(game);         break;
        case GAME_STATE_LEVEL_SELECT:  DrawLevelSelect(game);  break;
        case GAME_STATE_CONTROLS:      DrawControls();         break;
        case GAME_STATE_PLAYING:       DrawHUD(game);          break;
        default: break;
    }
}

/* ============================================================
 *  Application lifecycle
 * ============================================================ */

void Game_Run(Game *game) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);

    SetExitKey(KEY_NULL);
    LoadBackground(game);
    PlayerSprites_Load(&game->sprites);

    Game_Init(game);
    Editor_LoadAll(game->customLevels);
    LoadProgress(game);

    while (!WindowShouldClose() && !game->quitRequested) {
        switch (game->state) {
            case GAME_STATE_MENU:
                UpdateMenu(game);
                break;

            case GAME_STATE_LEVEL_SELECT:
                UpdateLevelSelect(game);
                break;

            case GAME_STATE_CONTROLS:
                UpdateControls(game);
                break;

            case GAME_STATE_EDITOR:
                Editor_Update(game);
                break;

            case GAME_STATE_PLAYING:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    game->state = GAME_STATE_MENU;
                } else if (game->active) {
                    Game_Update(game);
                } else if (IsKeyPressed(KEY_R)) {
                    Game_Restart(game);
                } else if (IsKeyPressed(KEY_M)) {
                    game->state = GAME_STATE_MENU;
                }
                break;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        Game_Draw(game);
        EndDrawing();
    }

    SaveProgress(game);
    Editor_SaveAll(game->customLevels);

    PlayerSprites_Unload(&game->sprites);
    if (game->background.id != 0) UnloadTexture(game->background);
    CloseWindow();
}