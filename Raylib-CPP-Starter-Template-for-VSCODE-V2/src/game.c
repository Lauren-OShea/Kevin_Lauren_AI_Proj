#include "game.h"
#include <math.h>
#include <string.h>

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
 *  Save / Load progress
 * ============================================================ */

#define SAVE_FILE "save.dat"

static void SaveProgress(const Game *game) {
    unsigned char buf[MAX_LEVELS];
    for (int i = 0; i < MAX_LEVELS; i++) {
        buf[i] = game->levelCompleted[i] ? 1 : 0;
    }
    if (!SaveFileData(SAVE_FILE, buf, MAX_LEVELS)) {
        TraceLog(LOG_WARNING, "Could not write %s", SAVE_FILE);
    }
}

static void LoadProgress(Game *game) {
    int size = 0;
    unsigned char *data = LoadFileData(SAVE_FILE, &size);

    if (!data) {
        TraceLog(LOG_INFO, "No save file found — starting fresh.");
        return;
    }

    if (size == MAX_LEVELS) {
        for (int i = 0; i < MAX_LEVELS; i++) {
            game->levelCompleted[i] = (data[i] != 0);
        }
        TraceLog(LOG_INFO, "Loaded %s — progress restored.", SAVE_FILE);
    } else {
        TraceLog(LOG_WARNING,
                 "Save file size mismatch (%d, expected %d). Ignored.",
                 size, MAX_LEVELS);
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

    DrawText(TextFormat("LEVEL %d / %d", game->currentLevel + 1, MAX_LEVELS),
             20, 52, 20, (Color){ 200, 220, 240, 220 });

    DrawLivesRow(game->players[0].lives, 30, 92,
                 (Color){ 120, 200, 255, 255 });

    int p2StartX = SCREEN_WIDTH - 30 - (JACK_MAX_LIVES - 1) * 26;
    DrawLivesRow(game->players[1].lives, p2StartX, 92,
                 (Color){ 255, 110, 110, 255 });

    float remaining = game->levelTimer;
    if (remaining < 0.0f) remaining = 0.0f;

    const char *timeStr = TextFormat("%.1f", remaining);
    int  tf = 54;
    int  tw = MeasureText(timeStr, tf);
    int  tx = SCREEN_WIDTH / 2 - tw / 2;
    int  ty = 14;

    Color timeCol;
    if (remaining <= 1.0f)      timeCol = (Color){ 255,  90,  90, 255 };
    else if (remaining <= 2.5f) timeCol = (Color){ 255, 180,  90, 255 };
    else                        timeCol = (Color){ 232, 245, 255, 255 };

    DrawText("SURVIVE", SCREEN_WIDTH / 2 - MeasureText("SURVIVE", 16) / 2,
             2, 16, (Color){ 200, 220, 240, 220 });
    DrawText(timeStr, tx + 2, ty + 2, tf, (Color){ 0, 0, 0, 120 });
    DrawText(timeStr, tx,     ty,     tf, timeCol);

    DrawText("P1: A/D  SPACE  E",
             20, SCREEN_HEIGHT - 26, 16, (Color){ 200, 220, 240, 200 });
    DrawText("P2: ARROWS  UP  M",
             SCREEN_WIDTH - 190, SCREEN_HEIGHT - 26, 16,
             (Color){ 240, 200, 220, 200 });

    if (game->levelCompleteTimer > 0.0f) {
        const char *msg = "LEVEL COMPLETE!";
        int fs = 48;
        DrawText(msg,
                 SCREEN_WIDTH / 2 - MeasureText(msg, fs) / 2,
                 SCREEN_HEIGHT / 2 - fs / 2, fs,
                 (Color){ 240, 250, 255, 240 });
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
    const LevelDef *L = &LEVELS[game->currentLevel];

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

    float viewW = (float)SCREEN_WIDTH  / z;
    float viewH = (float)SCREEN_HEIGHT / z;
    float halfW = viewW * 0.5f;
    float halfH = viewH * 0.5f;

    if (viewW >= L->worldW) center.x = L->worldW * 0.5f;
    else center.x = fmaxf(halfW, fminf(L->worldW - halfW, center.x));

    if (viewH >= L->worldH) center.y = L->worldH * 0.5f;
    else center.y = fmaxf(halfH, fminf(L->worldH - halfH, center.y));

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

    const LevelDef *L = &LEVELS[game->currentLevel];
    float viewW = (float)SCREEN_WIDTH  / game->camera.zoom;
    float viewH = (float)SCREEN_HEIGHT / game->camera.zoom;
    float halfW = viewW * 0.5f;
    float halfH = viewH * 0.5f;

    if (viewW >= L->worldW) game->camera.target.x = L->worldW * 0.5f;
    else game->camera.target.x = fmaxf(halfW, fminf(L->worldW - halfW,
                                                    game->camera.target.x));

    if (viewH >= L->worldH) game->camera.target.y = L->worldH * 0.5f;
    else game->camera.target.y = fmaxf(halfH, fminf(L->worldH - halfH,
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

#define LS_COLS    4
#define LS_ROWS    5
#define LS_BTN_W   130
#define LS_BTN_H   60
#define LS_GAP_X   20
#define LS_GAP_Y   16
#define LS_TOP     110

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

static bool LevelUnlocked(const Game *game, int idx) {
    if (idx == 0) return true;
    return game->levelCompleted[idx - 1];
}

static void UpdateLevelSelect(Game *game) {
    int col = game->levelSelectSelection % LS_COLS;
    int row = game->levelSelectSelection / LS_COLS;

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        col = (col + 1) % LS_COLS;
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        col = (col + LS_COLS - 1) % LS_COLS;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        row = (row + 1) % LS_ROWS;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        row = (row + LS_ROWS - 1) % LS_ROWS;
    }
    game->levelSelectSelection = row * LS_COLS + col;

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
        game->state = GAME_STATE_MENU;
        return;
    }

    Vector2 mouse = GetMousePosition();
    Vector2 delta = GetMouseDelta();
    bool    moved = (delta.x != 0.0f || delta.y != 0.0f);
    bool    hovered = false;

    for (int i = 0; i < MAX_LEVELS; i++) {
        if (CheckCollisionPointRec(mouse, LevelButtonRect(i))) {
            if (moved) game->levelSelectSelection = i;
            if (game->levelSelectSelection == i) hovered = true;
        }
    }

    bool activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                    (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));

    if (!activate) return;

    int idx = game->levelSelectSelection;
    if (LevelUnlocked(game, idx)) {
        Game_LoadLevel(game, idx);
    }
}

static void DrawLevelSelect(const Game *game) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 150 });

    const char *title = "LEVEL SELECT";
    DrawText(title,
             SCREEN_WIDTH / 2 - MeasureText(title, 42) / 2,
             45, 42, (Color){ 232, 245, 255, 255 });

    for (int i = 0; i < MAX_LEVELS; i++) {
        Rectangle r        = LevelButtonRect(i);
        bool      unlocked = LevelUnlocked(game, i);
        bool      done     = game->levelCompleted[i];
        bool      selected = (game->levelSelectSelection == i);

        Color fill, border, text;
        if (!unlocked) {
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
            fill   = (Color){ fill.r + 40, fill.g + 40, fill.b + 40, fill.a };
            border = (Color){ 255, 255, 255, 255 };
        }

        DrawRectangleRec(r, fill);
        DrawRectangleLinesEx(r, 2.5f, border);

        const char *num = TextFormat("%d", i + 1);
        int  fs = 28;
        DrawText(num,
                 (int)(r.x + r.width / 2) - MeasureText(num, fs) / 2,
                 (int)(r.y + 8),
                 fs, text);

        const char *name = LEVELS[i].name;
        int  ns = 12;
        if (!unlocked) name = "LOCKED";
        DrawText(name,
                 (int)(r.x + r.width / 2) - MeasureText(name, ns) / 2,
                 (int)(r.y + r.height - 20),
                 ns, text);
    }

    const char *hint = "Arrows to move  -  Enter to play  -  Esc to go back";
    DrawText(hint,
             SCREEN_WIDTH / 2 - MeasureText(hint, 15) / 2,
             SCREEN_HEIGHT - 24, 15, (Color){ 200, 220, 240, 200 });
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
        "Survive 5 seconds to complete the level.",
        "You have 3 lives. If both players are out, the level fails."
    };
    int count = (int)(sizeof(lines) / sizeof(lines[0]));

    for (int i = 0; i < count; i++) {
        DrawText(lines[i],
                 SCREEN_WIDTH / 2 - MeasureText(lines[i], 20) / 2,
                 170 + i * 40, 20, (Color){ 200, 225, 245, 255 });
    }

    const char *back = "Press Esc or click to go back";
    DrawText(back,
             SCREEN_WIDTH / 2 - MeasureText(back, 18) / 2,
             SCREEN_HEIGHT - 50, 18, (Color){ 170, 210, 240, 220 });
}

/* ============================================================
 *  Level loading / spawning
 * ============================================================ */

static void LoadLevelPlatforms(Game *game, const LevelDef *L) {
    Platform_InitAll(game->platforms);
    for (int i = 0; i < L->nPlatforms && i < MAX_PLATFORMS; i++) {
        Platform_Add(game->platforms, L->platforms[i]);
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

void Game_LoadLevel(Game *game, int idx) {
    if (idx < 0)             idx = 0;
    if (idx >= MAX_LEVELS)   idx = MAX_LEVELS - 1;

    game->currentLevel = idx;

    const LevelDef *L = &LEVELS[idx];

    LoadLevelPlatforms(game, L);

    Jack_Init(&game->players[0], L->spawn1, WHITE);
    Jack_Init(&game->players[1], L->spawn2, (Color){ 255, 110, 110, 255 });

    Shard_InitAll(game->shards);
    Enemy_InitAll(game->enemies);
    EnemyProjectile_InitAll(game->enemyProjectiles);

    SpawnEnemiesForLevel(game, L);

    game->active             = true;
    game->state              = GAME_STATE_PLAYING;
    game->frameCounter       = 0.0f;
    game->levelCompleteTimer = 0.0f;
    game->levelTimer         = 5.0f;

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

static const JackControls P1_CONTROLS = { KEY_A, KEY_D, KEY_SPACE };
static const JackControls P2_CONTROLS = { KEY_LEFT, KEY_RIGHT, KEY_UP };

void Game_Init(Game *game) {
    memset(game->levelCompleted, 0, sizeof(game->levelCompleted));
    game->currentLevel         = 0;
    game->menuSelection        = 0;
    game->levelSelectSelection = 0;
    game->quitRequested        = false;
    game->score                = 0;
    game->active               = false;
    game->levelCompleteTimer   = 0.0f;
    game->levelTimer           = 0.0f;
    game->state                = GAME_STATE_MENU;
    game->camera               = (Camera2D){ 0 };
}

void Game_Restart(Game *game) {
    Game_LoadLevel(game, game->currentLevel);
}

void Game_Update(Game *game) {
    if (!game->active) return;

    float dt = GetFrameTime();

    if (game->levelCompleteTimer > 0.0f) {
        game->levelCompleteTimer -= dt;
        if (game->levelCompleteTimer <= 0.0f) {
            game->levelCompleteTimer = 0.0f;
            int next = game->currentLevel + 1;
            if (next >= MAX_LEVELS) {
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

    const LevelDef *L = &LEVELS[game->currentLevel];

    Jack_Update(&game->players[0], game->platforms, &P1_CONTROLS,
                L->worldW, L->worldH);
    Jack_Update(&game->players[1], game->platforms, &P2_CONTROLS,
                L->worldW, L->worldH);

    Shard_UpdateAll(game->shards, L->worldW);
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
    for (int p = 0; p < PLAYER_COUNT; p++) {
        nullHitboxes[p] = (Rectangle){ -10000, -10000, 0, 0 };
    }
    Enemy_UpdateAll(game->enemies, playerPositions, nullHitboxes,
                    PLAYER_COUNT, game->enemyProjectiles);

    EnemyProjectile_UpdateAll(game->enemyProjectiles, nullHitboxes,
                              PLAYER_COUNT, L->worldW, L->worldH);

    ApplyPlayerThreatCollisions(game, playerHitboxes);

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

    /* Survival timer */
    game->levelTimer -= dt;
    if (game->levelTimer <= 0.0f) {
        game->levelTimer = 0.0f;
        if (!game->levelCompleted[game->currentLevel]) {
            game->levelCompleted[game->currentLevel] = true;
            SaveProgress(game);
        }
        game->levelCompleteTimer = 1.5f;
    }

    UpdateCameraForLevel(game, dt);
}

void Game_Draw(const Game *game) {
    DrawBackgroundImage(game);

    if (game->state == GAME_STATE_PLAYING) {
        BeginMode2D(game->camera);

            Platform_DrawAll(game->platforms);
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

    PlayerSprites_Unload(&game->sprites);
    if (game->background.id != 0) UnloadTexture(game->background);
    CloseWindow();
}