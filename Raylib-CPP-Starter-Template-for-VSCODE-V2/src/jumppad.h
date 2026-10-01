#ifndef JACKFROST_JUMPPAD_H
#define JACKFROST_JUMPPAD_H

#include <stdbool.h>
#include "raylib.h"

#define MAX_JUMPPADS        32
#define JUMPPAD_WIDTH       40
#define JUMPPAD_HEIGHT      12
#define JUMPPAD_LAUNCH_VY   18.0f
#define JUMPPAD_DETECT_PAD  20

typedef struct JumpPad {
    Rectangle bounds;
    float     animTimer;
    bool      active;
} JumpPad;

void JumpPad_InitAll   (JumpPad pads[MAX_JUMPPADS]);
bool JumpPad_Add       (JumpPad pads[MAX_JUMPPADS], Rectangle bounds);
void JumpPad_UpdateAll (JumpPad pads[MAX_JUMPPADS], float dt);
void JumpPad_DrawAll   (const JumpPad pads[MAX_JUMPPADS]);

bool JumpPad_TryTrigger(const JumpPad pads[MAX_JUMPPADS], Rectangle entityRect);

#endif /* JACKFROST_JUMPPAD_H */