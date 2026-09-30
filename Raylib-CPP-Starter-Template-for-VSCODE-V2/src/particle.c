#include "particle.h"
#include <math.h>
#include <stdlib.h>

void Particle_InitAll(ParticleSystem *ps) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        ps->items[i].active = false;
    }
}

void Particle_SpawnIcy(ParticleSystem *ps, Vector2 pos, float spreadX) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (ps->items[i].active) continue;

        Particle *p = &ps->items[i];

        /* Random spread using rand() — raylib's GetRandomValue is
           fine too, but rand() gives finer floats. */
        float rx = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        float ry = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;

        p->position  = pos;
        p->velocity  = (Vector2){
            rx * spreadX,
            -fabsf(ry) * 40.0f - 10.0f   /* mostly upward */
        };
        p->maxLife   = 0.4f + ((float)rand() / RAND_MAX) * 0.5f;
        p->life      = p->maxLife;
        p->size      = 1.5f + ((float)rand() / RAND_MAX) * 2.0f;
        p->active    = true;
        return;
    }
}

void Particle_UpdateAll(ParticleSystem *ps, float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ps->items[i].active) continue;

        Particle *p = &ps->items[i];

        p->position.x += p->velocity.x * dt;
        p->position.y += p->velocity.y * dt;
        p->velocity.y += 40.0f * dt;   /* slight gravity */
        p->life       -= dt;

        if (p->life <= 0.0f) p->active = false;
    }
}

void Particle_DrawAll(const ParticleSystem *ps) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ps->items[i].active) continue;

        const Particle *p = &ps->items[i];

        float t = p->life / p->maxLife;
        if (t < 0.0f) t = 0.0f;

        unsigned char alpha = (unsigned char)(t * 220.0f);
        Color c = (Color){ 200, 240, 255, alpha };

        DrawCircleV(p->position, p->size, c);
        DrawCircleV(p->position, p->size * 0.5f,
                    (Color){ 255, 255, 255, alpha });
    }
}