#include "editor.h"
#include "game.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

#define CUSTOM_SAVE_FILE "custom_levels.dat"

/* ============================================================
 *  Save / load
 * ============================================================ */

void Editor_InitCustomLevel(CustomLevel *lvl) {
    memset(lvl, 0, sizeof(CustomLevel));
    lvl->used   = false;
    lvl->spawn1 = (Vector2){ 80,  400 };
    lvl->spawn2 = (Vector2){ 140, 400 };
    strncpy(lvl->name, "Custom", sizeof(lvl->name) - 1);
}

/* Clear everything the player has placed. Because a level with no
 * platforms is considered empty, also clear the `used` flag so the
 * slot reverts to the "CREATE" state in the level select. */
static void Editor_ClearLevelContent(CustomLevel *lvl) {
    char nameCopy[24];
    strncpy(nameCopy, lvl->name, sizeof(nameCopy) - 1);
    nameCopy[sizeof(nameCopy) - 1] = '\0';

    Editor_InitCustomLevel(lvl);

    lvl->used = false;
    strncpy(lvl->name, nameCopy, sizeof(lvl->name) - 1);
    lvl->name[sizeof(lvl->name) - 1] = '\0';
}

void Editor_SaveAll(const CustomLevel levels[MAX_CUSTOM_LEVELS]) {
    if (!SaveFileData(CUSTOM_SAVE_FILE, (void*)levels,
                      sizeof(CustomLevel) * MAX_CUSTOM_LEVELS)) {
        TraceLog(LOG_WARNING, "Could not write %s", CUSTOM_SAVE_FILE);
    }
}

bool Editor_LoadAll(CustomLevel levels[MAX_CUSTOM_LEVELS]) {
    int size = 0;
    unsigned char *data = LoadFileData(CUSTOM_SAVE_FILE, &size);
    if (!data) return false;

    size_t expected = sizeof(CustomLevel) * MAX_CUSTOM_LEVELS;
    if ((size_t)size != expected) {
        TraceLog(LOG_WARNING,
                 "Custom level save size mismatch (%d, expected %zu)",
                 size, expected);
        UnloadFileData(data);
        return false;
    }
    memcpy(levels, data, size);
    UnloadFileData(data);
    return true;
}

void Editor_Enter(EditorState *e, int slot) {
    if (slot < 0) slot = 0;
    if (slot >= MAX_CUSTOM_LEVELS) slot = MAX_CUSTOM_LEVELS - 1;

    e->active         = true;
    e->slot           = slot;
    e->tool           = TOOL_PLATFORM;
    e->enemyKind      = 0;
    e->dragging       = false;
    e->movingHasStart = false;
    e->confirmReset   = false;
    e->status[0]      = '\0';
    e->statusTimer    = 0.0f;

    float fitZoom = fminf((float)SCREEN_WIDTH  / EDITOR_WORLD_W,
                          (float)SCREEN_HEIGHT / EDITOR_WORLD_H);
    if (fitZoom > 1.0f) fitZoom = 1.0f;

    e->cam.zoom     = fitZoom;
    e->cam.target   = (Vector2){ EDITOR_WORLD_W * 0.5f,
                                 EDITOR_WORLD_H * 0.5f };
    e->cam.offset   = (Vector2){ SCREEN_WIDTH  * 0.5f,
                                 SCREEN_HEIGHT * 0.5f };
    e->cam.rotation = 0.0f;
}

/* ============================================================
 *  Helpers
 * ============================================================ */

static void SetStatus(EditorState *e, const char *msg) {
    strncpy(e->status, msg, sizeof(e->status) - 1);
    e->status[sizeof(e->status) - 1] = '\0';
    e->statusTimer = 2.0f;
}

static Vector2 SnapToGrid(Vector2 p) {
    return (Vector2){
        floorf(p.x / EDITOR_GRID) * EDITOR_GRID,
        floorf(p.y / EDITOR_GRID) * EDITOR_GRID
    };
}

static Rectangle MakeRect(Vector2 a, Vector2 b) {
    float x0 = fminf(a.x, b.x);
    float y0 = fminf(a.y, b.y);
    float x1 = fmaxf(a.x, b.x);
    float y1 = fmaxf(a.y, b.y);
    return (Rectangle){ x0, y0, x1 - x0, y1 - y0 };
}

/* Find the platform under the cursor that the enemy should stand on. */
static Rectangle *FindPlatformAt(CustomLevel *lvl, Vector2 world, float *outTop) {
    Rectangle *best = NULL;
    float bestDist = 1e9f;

    for (int i = 0; i < lvl->nPlatforms; i++) {
        Rectangle p = lvl->platforms[i];
        if (world.x < p.x || world.x > p.x + p.width) continue;
        if (p.y < world.y) continue;

        float dist = p.y - world.y;
        if (dist < bestDist) {
            bestDist = dist;
            best = &lvl->platforms[i];
        }
    }
    if (best) *outTop = best->y;
    return best;
}

/* Delete-target hit testing */
typedef enum {
    DEL_NONE = 0,
    DEL_PLATFORM,
    DEL_LADDER,
    DEL_SPIKE,
    DEL_JUMPPAD,
    DEL_MOVING,
    DEL_ENEMY
} DelKind;

typedef struct {
    DelKind  kind;
    int      idx;
    float    area;
} DelHit;

static DelHit FindDeleteTarget(const CustomLevel *lvl, Vector2 world) {
    DelHit hit = { DEL_NONE, -1, 1e18f };

    for (int i = 0; i < lvl->nPlatforms; i++) {
        if (CheckCollisionPointRec(world, lvl->platforms[i])) {
            float a = lvl->platforms[i].width * lvl->platforms[i].height;
            if (a < hit.area) { hit.kind = DEL_PLATFORM; hit.idx = i; hit.area = a; }
        }
    }
    for (int i = 0; i < lvl->nLadders; i++) {
        if (CheckCollisionPointRec(world, lvl->ladders[i])) {
            float a = lvl->ladders[i].width * lvl->ladders[i].height;
            if (a < hit.area) { hit.kind = DEL_LADDER; hit.idx = i; hit.area = a; }
        }
    }
    for (int i = 0; i < lvl->nSpikes; i++) {
        if (CheckCollisionPointRec(world, lvl->spikes[i])) {
            float a = lvl->spikes[i].width * lvl->spikes[i].height;
            if (a < hit.area) { hit.kind = DEL_SPIKE; hit.idx = i; hit.area = a; }
        }
    }
    for (int i = 0; i < lvl->nJumppads; i++) {
        if (CheckCollisionPointRec(world, lvl->jumppads[i])) {
            float a = lvl->jumppads[i].width * lvl->jumppads[i].height;
            if (a < hit.area) { hit.kind = DEL_JUMPPAD; hit.idx = i; hit.area = a; }
        }
    }
    for (int i = 0; i < lvl->nMoving; i++) {
        if (CheckCollisionPointRec(world, lvl->moving[i].startRect) ||
            CheckCollisionPointRec(world, lvl->moving[i].endRect)) {
            float a = lvl->moving[i].startRect.width *
                      lvl->moving[i].startRect.height;
            if (a < hit.area) { hit.kind = DEL_MOVING; hit.idx = i; hit.area = a; }
        }
    }
    for (int i = 0; i < lvl->nEnemies; i++) {
        Rectangle er = {
            lvl->enemies[i].position.x - ENEMY_WIDTH  * 0.5f,
            lvl->enemies[i].position.y - ENEMY_HEIGHT,
            (float)ENEMY_WIDTH, (float)ENEMY_HEIGHT
        };
        if (CheckCollisionPointRec(world, er)) {
            float a = er.width * er.height;
            if (a < hit.area) { hit.kind = DEL_ENEMY; hit.idx = i; hit.area = a; }
        }
    }
    return hit;
}

static void DeleteAt(CustomLevel *lvl, int idx, DelKind kind) {
    switch (kind) {
        case DEL_PLATFORM:
            for (int i = idx; i < lvl->nPlatforms - 1; i++)
                lvl->platforms[i] = lvl->platforms[i + 1];
            if (lvl->nPlatforms > 0) lvl->nPlatforms--;
            break;
        case DEL_LADDER:
            for (int i = idx; i < lvl->nLadders - 1; i++)
                lvl->ladders[i] = lvl->ladders[i + 1];
            if (lvl->nLadders > 0) lvl->nLadders--;
            break;
        case DEL_SPIKE:
            for (int i = idx; i < lvl->nSpikes - 1; i++)
                lvl->spikes[i] = lvl->spikes[i + 1];
            if (lvl->nSpikes > 0) lvl->nSpikes--;
            break;
        case DEL_JUMPPAD:
            for (int i = idx; i < lvl->nJumppads - 1; i++)
                lvl->jumppads[i] = lvl->jumppads[i + 1];
            if (lvl->nJumppads > 0) lvl->nJumppads--;
            break;
        case DEL_MOVING:
            for (int i = idx; i < lvl->nMoving - 1; i++)
                lvl->moving[i] = lvl->moving[i + 1];
            if (lvl->nMoving > 0) lvl->nMoving--;
            break;
        case DEL_ENEMY:
            for (int i = idx; i < lvl->nEnemies - 1; i++)
                lvl->enemies[i] = lvl->enemies[i + 1];
            if (lvl->nEnemies > 0) lvl->nEnemies--;
            break;
        default: break;
    }
}

/* ============================================================
 *  Confirmation dialog — shared layout between update & draw
 * ============================================================ */

static Rectangle ResetDialogPanel(void) {
    return (Rectangle){
        (SCREEN_WIDTH  - 460) / 2.0f,
        (SCREEN_HEIGHT - 200) / 2.0f,
        460.0f, 200.0f
    };
}

static Rectangle ResetDialogConfirmBtn(void) {
    Rectangle p = ResetDialogPanel();
    return (Rectangle){ SCREEN_WIDTH / 2.0f - 150.0f,
                        p.y + 140.0f, 130.0f, 32.0f };
}

static Rectangle ResetDialogCancelBtn(void) {
    Rectangle p = ResetDialogPanel();
    return (Rectangle){ SCREEN_WIDTH / 2.0f + 20.0f,
                        p.y + 140.0f, 130.0f, 32.0f };
}

/* ============================================================
 *  Placement finalizers
 * ============================================================ */

static void FinalizeDrag(EditorState *e, CustomLevel *lvl) {
    Rectangle r = MakeRect(e->dragStart, e->dragEnd);

    if (e->tool == TOOL_PLATFORM) {
        if (r.width < EDITOR_GRID)     r.width = EDITOR_GRID;
        if (r.height < EDITOR_GRID)    r.height = EDITOR_GRID;
        if (lvl->nPlatforms < EDITOR_MAX_PLATFORMS) {
            lvl->platforms[lvl->nPlatforms++] = r;
            SetStatus(e, "Platform placed");
        } else {
            SetStatus(e, "Platform limit reached");
        }
    } else if (e->tool == TOOL_LADDER) {
        r.width = LADDER_WIDTH;
        if (r.height < EDITOR_GRID * 2) r.height = EDITOR_GRID * 2;
        if (lvl->nLadders < EDITOR_MAX_LADDERS) {
            lvl->ladders[lvl->nLadders++] = r;
            SetStatus(e, "Ladder placed");
        } else {
            SetStatus(e, "Ladder limit reached");
        }
    }
}

static void PlaceSingle(EditorState *e, CustomLevel *lvl, Vector2 world) {
    Vector2 snapped = SnapToGrid(world);

    switch (e->tool) {
        case TOOL_SPIKE: {
            if (lvl->nSpikes >= EDITOR_MAX_SPIKES) {
                SetStatus(e, "Spike limit reached"); return;
            }
            Rectangle r = { snapped.x, snapped.y,
                            SPIKE_WIDTH, SPIKE_HEIGHT };
            lvl->spikes[lvl->nSpikes++] = r;
            SetStatus(e, "Spike placed");
        } break;

        case TOOL_JUMPPAD: {
            if (lvl->nJumppads >= EDITOR_MAX_JUMPPADS) {
                SetStatus(e, "Jumppad limit reached"); return;
            }
            Rectangle r = { snapped.x, snapped.y,
                            JUMPPAD_WIDTH, JUMPPAD_HEIGHT };
            lvl->jumppads[lvl->nJumppads++] = r;
            SetStatus(e, "Jumppad placed");
        } break;

        case TOOL_ENEMY: {
            if (lvl->nEnemies >= EDITOR_MAX_ENEMIES) {
                SetStatus(e, "Enemy limit reached"); return;
            }

            EditorEnemy en;
            en.type = e->enemyKind;

            if (en.type == 3 /* flyer */) {
                en.position    = (Vector2){ snapped.x, snapped.y };
                en.baseY       = snapped.y;
                en.patrolLeft  = snapped.x - 40.0f;
                en.patrolRight = snapped.x + 40.0f;
            } else {
                float platformTop = snapped.y;
                Rectangle *onPlat = FindPlatformAt(lvl, world, &platformTop);

                if (onPlat) {
                    en.patrolLeft  = onPlat->x + 12.0f;
                    en.patrolRight = onPlat->x + onPlat->width - 12.0f;
                    if (en.patrolRight - en.patrolLeft < 40.0f) {
                        float c = (en.patrolLeft + en.patrolRight) * 0.5f;
                        en.patrolLeft  = c - 20.0f;
                        en.patrolRight = c + 20.0f;
                    }
                    en.baseY    = onPlat->y;
                    en.position = (Vector2){ snapped.x, onPlat->y };
                } else {
                    en.patrolLeft  = snapped.x - 50.0f;
                    en.patrolRight = snapped.x + 50.0f;
                    en.baseY       = snapped.y;
                    en.position    = (Vector2){ snapped.x, snapped.y };
                }
            }

            lvl->enemies[lvl->nEnemies++] = en;
            SetStatus(e, "Enemy placed");
        } break;

        case TOOL_SPAWN1:
            lvl->spawn1 = snapped;
            SetStatus(e, "Player 1 spawn set");
            break;
        case TOOL_SPAWN2:
            lvl->spawn2 = snapped;
            SetStatus(e, "Player 2 spawn set");
            break;

        default: break;
    }
}

/* ============================================================
 *  Update
 * ============================================================ */

void Editor_Update(Game *game) {
    EditorState *e   = &game->editor;
    CustomLevel *lvl = &game->customLevels[e->slot];

    /* ============================================================
     *  Confirmation dialog — takes absolute priority.
     * ============================================================ */
    if (e->confirmReset) {
        Vector2 mouse = GetMousePosition();
        bool    click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        bool hoverConfirm = CheckCollisionPointRec(mouse,
                                                   ResetDialogConfirmBtn());
        bool hoverCancel  = CheckCollisionPointRec(mouse,
                                                   ResetDialogCancelBtn());

        bool confirm = IsKeyPressed(KEY_Y) || IsKeyPressed(KEY_ENTER) ||
                       (hoverConfirm && click);
        bool cancel  = IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ESCAPE) ||
                       (hoverCancel  && click);

        if (confirm) {
            Editor_ClearLevelContent(lvl);
            e->dragging       = false;
            e->movingHasStart = false;
            e->confirmReset   = false;
            SetStatus(e, "Level reset");
        } else if (cancel) {
            e->confirmReset = false;
            SetStatus(e, "Reset cancelled");
        }
        return;
    }

    /* ============================================================
     *  Reset key — open the confirmation dialog
     * ============================================================ */
    if (IsKeyPressed(KEY_DELETE)) {
        e->confirmReset = true;
        return;
    }

    /* -------- Pan -------- */
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 d = GetMouseDelta();
        e->cam.target.x -= d.x / e->cam.zoom;
        e->cam.target.y -= d.y / e->cam.zoom;
    }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  e->cam.target.x -= 6.0f / e->cam.zoom;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) e->cam.target.x += 6.0f / e->cam.zoom;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    e->cam.target.y -= 6.0f / e->cam.zoom;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  e->cam.target.y += 6.0f / e->cam.zoom;

    /* -------- Zoom -------- */
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        e->cam.zoom += wheel * 0.25f;
        if (e->cam.zoom < 0.25f) e->cam.zoom = 0.25f;
        if (e->cam.zoom > 2.0f)  e->cam.zoom = 2.0f;
        e->cam.zoom = roundf(e->cam.zoom * 4.0f) / 4.0f;
    }

    float viewW = SCREEN_WIDTH  / e->cam.zoom;
    float viewH = SCREEN_HEIGHT / e->cam.zoom;
    if (viewW >= EDITOR_WORLD_W) e->cam.target.x = EDITOR_WORLD_W * 0.5f;
    else e->cam.target.x = fmaxf(viewW * 0.5f,
                                 fminf(EDITOR_WORLD_W - viewW * 0.5f,
                                       e->cam.target.x));
    if (viewH >= EDITOR_WORLD_H) e->cam.target.y = EDITOR_WORLD_H * 0.5f;
    else e->cam.target.y = fmaxf(viewH * 0.5f,
                                 fminf(EDITOR_WORLD_H - viewH * 0.5f,
                                       e->cam.target.y));

    /* -------- Tool selection -------- */
    if (IsKeyPressed(KEY_ONE))   e->tool = TOOL_PLATFORM;
    if (IsKeyPressed(KEY_TWO))   e->tool = TOOL_LADDER;
    if (IsKeyPressed(KEY_THREE)) e->tool = TOOL_SPIKE;
    if (IsKeyPressed(KEY_FOUR))  e->tool = TOOL_JUMPPAD;
    if (IsKeyPressed(KEY_FIVE))  e->tool = TOOL_MOVING;
    if (IsKeyPressed(KEY_SIX))   e->tool = TOOL_ENEMY;
    if (IsKeyPressed(KEY_SEVEN)) e->tool = TOOL_SPAWN1;
    if (IsKeyPressed(KEY_EIGHT)) e->tool = TOOL_SPAWN2;

    if (IsKeyPressed(KEY_Q)) e->enemyKind = (e->enemyKind + 4) % 5;
    if (IsKeyPressed(KEY_R)) e->enemyKind = (e->enemyKind + 1) % 5;

    if (e->statusTimer > 0.0f) {
        e->statusTimer -= GetFrameTime();
        if (e->statusTimer <= 0.0f) e->status[0] = '\0';
    }

    /* -------- Exit / Playtest -------- */
    if (IsKeyPressed(KEY_ESCAPE)) {
        lvl->used = (lvl->nPlatforms > 0);
        snprintf(lvl->name, sizeof(lvl->name), "Custom %d", e->slot + 1);
        Editor_SaveAll(game->customLevels);
        game->state = GAME_STATE_LEVEL_SELECT;
        e->active   = false;
        return;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        lvl->used = (lvl->nPlatforms > 0);
        snprintf(lvl->name, sizeof(lvl->name), "Custom %d", e->slot + 1);
        Editor_SaveAll(game->customLevels);

        int idx = MAX_LEVELS + e->slot;
        e->active = false;
        Game_LoadLevel(game, idx);
        return;
    }

    /* -------- Mouse in world -------- */
    Vector2 mouseScreen = GetMousePosition();
    Vector2 mouseWorld  = GetScreenToWorld2D(mouseScreen, e->cam);
    Vector2 snapped     = SnapToGrid(mouseWorld);

    /* -------- Right-click delete -------- */
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (e->tool == TOOL_MOVING && e->movingHasStart) {
            e->movingHasStart = false;
            SetStatus(e, "Moving placement cancelled");
        } else {
            DelHit hit = FindDeleteTarget(lvl, mouseWorld);
            if (hit.kind != DEL_NONE) {
                DeleteAt(lvl, hit.idx, hit.kind);
                SetStatus(e, "Deleted");
            }
        }
    }

    /* -------- Left-click actions per tool -------- */
    bool clickDown = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool clickUp   = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    switch (e->tool) {
        case TOOL_PLATFORM:
        case TOOL_LADDER:
            if (clickDown) {
                e->dragging  = true;
                e->dragStart = snapped;
                e->dragEnd   = snapped;
            }
            if (e->dragging) e->dragEnd = snapped;
            if (e->dragging && clickUp) {
                FinalizeDrag(e, lvl);
                e->dragging = false;
            }
            break;

        case TOOL_MOVING:
            if (clickDown) {
                if (!e->movingHasStart) {
                    e->movingStart    = (Rectangle){ snapped.x, snapped.y, 80, 16 };
                    e->movingHasStart = true;
                    SetStatus(e, "Click again to set end position");
                } else {
                    if (lvl->nMoving < EDITOR_MAX_MOVING) {
                        EditorMoving m;
                        m.startRect = e->movingStart;
                        m.endRect   = (Rectangle){ snapped.x, snapped.y, 80, 16 };
                        lvl->moving[lvl->nMoving++] = m;
                        SetStatus(e, "Moving platform placed");
                    } else {
                        SetStatus(e, "Moving platform limit reached");
                    }
                    e->movingHasStart = false;
                }
            }
            break;

        case TOOL_SPIKE:
        case TOOL_JUMPPAD:
        case TOOL_ENEMY:
        case TOOL_SPAWN1:
        case TOOL_SPAWN2:
            if (clickDown) PlaceSingle(e, lvl, mouseWorld);
            break;

        default: break;
    }
}

/* ============================================================
 *  Draw
 * ============================================================ */

static void DrawEditorGrid(const EditorState *e) {
    float x0 = e->cam.target.x - (SCREEN_WIDTH  * 0.5f) / e->cam.zoom;
    float y0 = e->cam.target.y - (SCREEN_HEIGHT * 0.5f) / e->cam.zoom;
    float x1 = e->cam.target.x + (SCREEN_WIDTH  * 0.5f) / e->cam.zoom;
    float y1 = e->cam.target.y + (SCREEN_HEIGHT * 0.5f) / e->cam.zoom;

    int gx0 = (int)floorf(x0 / EDITOR_GRID) * EDITOR_GRID;
    int gy0 = (int)floorf(y0 / EDITOR_GRID) * EDITOR_GRID;

    for (int x = gx0; x <= (int)x1; x += EDITOR_GRID) {
        DrawLineV((Vector2){ (float)x, y0 }, (Vector2){ (float)x, y1 },
                  (Color){ 60, 90, 130, 60 });
    }
    for (int y = gy0; y <= (int)y1; y += EDITOR_GRID) {
        DrawLineV((Vector2){ x0, (float)y }, (Vector2){ x1, (float)y },
                  (Color){ 60, 90, 130, 60 });
    }
}

static void DrawWorldBounds(void) {
    Rectangle r = { 0, 0, EDITOR_WORLD_W, EDITOR_WORLD_H };
    DrawRectangleLinesEx(r, 3.0f, (Color){ 200, 220, 255, 220 });
}

static void DrawPlacedElements(const CustomLevel *lvl) {
    if (lvl->nPlatforms > 0) {
        Platform temp[MAX_PLATFORMS];
        Platform_InitAll(temp);
        for (int i = 0; i < lvl->nPlatforms && i < MAX_PLATFORMS; i++)
            Platform_Add(temp, lvl->platforms[i]);
        Platform_DrawAll(temp, NULL);
    }

    if (lvl->nLadders > 0) {
        Ladder temp[MAX_LADDERS];
        Ladder_InitAll(temp);
        for (int i = 0; i < lvl->nLadders && i < MAX_LADDERS; i++)
            Ladder_Add(temp, lvl->ladders[i]);
        Ladder_DrawAll(temp, NULL);
    }

    if (lvl->nSpikes > 0) {
        Spike temp[MAX_SPIKES];
        Spike_InitAll(temp);
        for (int i = 0; i < lvl->nSpikes && i < MAX_SPIKES; i++)
            Spike_Add(temp, lvl->spikes[i]);
        Spike_DrawAll(temp);
    }

    if (lvl->nJumppads > 0) {
        JumpPad temp[MAX_JUMPPADS];
        JumpPad_InitAll(temp);
        for (int i = 0; i < lvl->nJumppads && i < MAX_JUMPPADS; i++)
            JumpPad_Add(temp, lvl->jumppads[i]);
        JumpPad_UpdateAll(temp, GetFrameTime());
        JumpPad_DrawAll(temp);
    }

    for (int i = 0; i < lvl->nMoving; i++) {
        Rectangle a = lvl->moving[i].startRect;
        Rectangle b = lvl->moving[i].endRect;

        Vector2 ca = { a.x + a.width * 0.5f, a.y + a.height * 0.5f };
        Vector2 cb = { b.x + b.width * 0.5f, b.y + b.height * 0.5f };

        DrawLineEx(ca, cb, 2.0f, (Color){ 160, 200, 255, 140 });

        DrawRectangleRec(a, (Color){  60,  90, 130, 180 });
        DrawRectangleLinesEx(a, 2.0f, (Color){ 180, 220, 255, 255 });
        DrawRectangleRec(b, (Color){ 100, 140, 200, 100 });
        DrawRectangleLinesEx(b, 2.0f, (Color){ 180, 220, 255, 200 });
    }

    if (lvl->nEnemies > 0) {
        Enemy temp[MAX_ENEMIES];
        Enemy_InitAll(temp);
        for (int i = 0; i < lvl->nEnemies && i < MAX_ENEMIES; i++) {
            EditorEnemy *se = &lvl->enemies[i];
            Enemy_PlaceDirect(temp, se->position, se->baseY,
                              se->patrolLeft, se->patrolRight,
                              (EnemyType)se->type);
        }
        Enemy_DrawAll(temp);
    }

    DrawCircle((int)lvl->spawn1.x, (int)lvl->spawn1.y, 14,
               (Color){ 120, 200, 255, 200 });
    DrawCircleLines((int)lvl->spawn1.x, (int)lvl->spawn1.y, 14,
                    (Color){ 255, 255, 255, 255 });
    DrawText("P1",
             (int)lvl->spawn1.x - MeasureText("P1", 14) / 2,
             (int)lvl->spawn1.y - 7, 14, (Color){ 15, 30, 50, 255 });

    DrawCircle((int)lvl->spawn2.x, (int)lvl->spawn2.y, 14,
               (Color){ 255, 110, 110, 200 });
    DrawCircleLines((int)lvl->spawn2.x, (int)lvl->spawn2.y, 14,
                    (Color){ 255, 255, 255, 255 });
    DrawText("P2",
             (int)lvl->spawn2.x - MeasureText("P2", 14) / 2,
             (int)lvl->spawn2.y - 7, 14, (Color){ 50, 15, 15, 255 });
}

static void DrawDragPreview(const EditorState *e) {
    if (!e->dragging) return;

    Rectangle r = MakeRect(e->dragStart, e->dragEnd);
    if (e->tool == TOOL_LADDER) r.width = LADDER_WIDTH;

    DrawRectangleRec(r, (Color){ 120, 190, 240, 90 });
    DrawRectangleLinesEx(r, 2.0f, (Color){ 200, 240, 255, 255 });
}

static void DrawMovingPreview(const EditorState *e, Vector2 mouseWorld) {
    if (e->tool != TOOL_MOVING || !e->movingHasStart) return;

    Vector2 snapped = SnapToGrid(mouseWorld);

    Rectangle a = e->movingStart;
    Rectangle b = { snapped.x, snapped.y, a.width, a.height };

    Vector2 ca = { a.x + a.width * 0.5f, a.y + a.height * 0.5f };
    Vector2 cb = { b.x + b.width * 0.5f, b.y + b.height * 0.5f };

    DrawLineEx(ca, cb, 2.0f, (Color){ 255, 220, 120, 220 });

    DrawRectangleRec(a, (Color){  60,  90, 130, 180 });
    DrawRectangleLinesEx(a, 2.0f, (Color){ 180, 220, 255, 255 });
    DrawRectangleLinesEx(b, 2.0f, (Color){ 255, 230, 140, 220 });
}

static void DrawCursorGhost(const EditorState *e, Vector2 mouseWorld) {
    Vector2 snapped = SnapToGrid(mouseWorld);

    switch (e->tool) {
        case TOOL_SPIKE: {
            Rectangle r = { snapped.x, snapped.y, SPIKE_WIDTH, SPIKE_HEIGHT };
            DrawRectangleLinesEx(r, 1.5f, (Color){ 255, 220, 220, 200 });
        } break;
        case TOOL_JUMPPAD: {
            Rectangle r = { snapped.x, snapped.y, JUMPPAD_WIDTH, JUMPPAD_HEIGHT };
            DrawRectangleLinesEx(r, 1.5f, (Color){ 200, 240, 255, 200 });
        } break;
        case TOOL_LADDER: {
            Rectangle r = { snapped.x, snapped.y, LADDER_WIDTH, 60.0f };
            DrawRectangleLinesEx(r, 1.5f, (Color){ 255, 220, 180, 200 });
        } break;
        case TOOL_ENEMY: {
            DrawCircleLines((int)snapped.x, (int)snapped.y, 12,
                            (Color){ 255, 220, 180, 200 });
        } break;
        case TOOL_SPAWN1: {
            DrawCircleLines((int)snapped.x, (int)snapped.y, 14,
                            (Color){ 180, 220, 255, 220 });
        } break;
        case TOOL_SPAWN2: {
            DrawCircleLines((int)snapped.x, (int)snapped.y, 14,
                            (Color){ 255, 180, 180, 220 });
        } break;
        default: break;
    }
}

static const char *TOOL_NAMES[TOOL_COUNT] = {
    "PLATFORM", "LADDER", "SPIKE", "JUMPPAD",
    "MOVING",   "ENEMY",  "SPAWN P1", "SPAWN P2"
};

static const char *ENEMY_KIND_NAMES[5] = {
    "WALKER", "SHOOTER", "DASHER", "FLYER", "UNSTUNNABLE"
};

static void DrawEditorTopBar(const EditorState *e) {
    DrawRectangle(0, 0, SCREEN_WIDTH, 36, (Color){ 0, 0, 0, 180 });

    const char *line = TextFormat("TOOL: %s   [1-8]   ENEMY: %s   [Q/R]",
                                  TOOL_NAMES[e->tool],
                                  ENEMY_KIND_NAMES[e->enemyKind]);
    DrawText(line, 12, 10, 18, (Color){ 232, 245, 255, 255 });

    const char *hint =
        "LMB place  RMB delete  WASD/MMB pan  Wheel zoom  DEL reset  ENTER play  ESC exit";
    DrawText(hint,
             SCREEN_WIDTH - MeasureText(hint, 13) - 12,
             SCREEN_HEIGHT - 22, 13, (Color){ 200, 220, 240, 220 });
}

static void DrawEditorCounts(const EditorState *e, const CustomLevel *lvl) {
    const char *line = TextFormat(
        "Plat %d/%d   Ldr %d/%d   Spk %d/%d   Pad %d/%d   Mov %d/%d   Enm %d/%d",
        lvl->nPlatforms, EDITOR_MAX_PLATFORMS,
        lvl->nLadders,   EDITOR_MAX_LADDERS,
        lvl->nSpikes,    EDITOR_MAX_SPIKES,
        lvl->nJumppads,  EDITOR_MAX_JUMPPADS,
        lvl->nMoving,    EDITOR_MAX_MOVING,
        lvl->nEnemies,   EDITOR_MAX_ENEMIES);
    DrawText(line, 12, 40, 14, (Color){ 200, 220, 240, 220 });
    (void)e;
}

/* ============================================================
 *  Confirmation dialog drawing
 * ============================================================ */

static void DrawResetConfirmDialog(void) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                  (Color){ 0, 0, 0, 180 });

    Rectangle panel   = ResetDialogPanel();
    Rectangle btnYes  = ResetDialogConfirmBtn();
    Rectangle btnNo   = ResetDialogCancelBtn();

    DrawRectangleRec(panel, (Color){ 30, 50, 80, 245 });
    DrawRectangleLinesEx(panel, 3.0f, (Color){ 220, 240, 255, 255 });

    const char *title = "RESET LEVEL?";
    DrawText(title,
             SCREEN_WIDTH / 2 - MeasureText(title, 32) / 2,
             (int)panel.y + 25, 32,
             (Color){ 255, 200, 200, 255 });

    const char *line1 = "This will delete everything";
    const char *line2 = "you've placed in this level.";
    DrawText(line1,
             SCREEN_WIDTH / 2 - MeasureText(line1, 18) / 2,
             (int)panel.y + 75, 18, (Color){ 220, 230, 240, 255 });
    DrawText(line2,
             SCREEN_WIDTH / 2 - MeasureText(line2, 18) / 2,
             (int)panel.y + 98, 18, (Color){ 220, 230, 240, 255 });

    Vector2 mouse = GetMousePosition();
    bool hoverYes = CheckCollisionPointRec(mouse, btnYes);
    bool hoverNo  = CheckCollisionPointRec(mouse, btnNo);

    DrawRectangleRec(btnYes,
                     hoverYes ? (Color){ 220,  80,  80, 255 }
                              : (Color){ 140,  45,  45, 230 });
    DrawRectangleLinesEx(btnYes, 2.0f, (Color){ 255, 230, 230, 255 });

    DrawRectangleRec(btnNo,
                     hoverNo ? (Color){  90, 150, 210, 255 }
                             : (Color){  40,  80, 130, 230 });
    DrawRectangleLinesEx(btnNo, 2.0f, (Color){ 200, 225, 255, 255 });

    const char *yesText = "[Y] Confirm";
    const char *noText  = "[N] Cancel";

    DrawText(yesText,
             (int)(btnYes.x + btnYes.width / 2) - MeasureText(yesText, 18) / 2,
             (int)(btnYes.y + (btnYes.height - 18) / 2),
             18, WHITE);

    DrawText(noText,
             (int)(btnNo.x + btnNo.width / 2) - MeasureText(noText, 18) / 2,
             (int)(btnNo.y + (btnNo.height - 18) / 2),
             18, WHITE);
}

void Editor_Draw(const Game *game) {
    const EditorState *e   = &game->editor;
    const CustomLevel *lvl = &game->customLevels[e->slot];

    BeginMode2D(e->cam);

        DrawWorldBounds();
        DrawEditorGrid(e);
        DrawPlacedElements(lvl);
        DrawDragPreview(e);
        DrawMovingPreview(e, GetScreenToWorld2D(GetMousePosition(), e->cam));
        DrawCursorGhost (e, GetScreenToWorld2D(GetMousePosition(), e->cam));

    EndMode2D();

    DrawEditorTopBar(e);
    DrawEditorCounts(e, lvl);

    if (e->statusTimer > 0.0f) {
        DrawText(e->status,
                 SCREEN_WIDTH / 2 - MeasureText(e->status, 22) / 2,
                 56, 22, (Color){ 240, 250, 255, 255 });
    }

    if (e->confirmReset) {
        DrawResetConfirmDialog();
    }
}