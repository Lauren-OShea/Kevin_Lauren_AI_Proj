#ifndef PARTICLE_H
#define PARTICLE_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_PARTICLES 256

typedef struct Particle {
    Vector2 position;
    Vector2 velocity;
    float   life;       /* seconds remaining */
    float   maxLife;
    float   size;
    bool    active;
} Particle;

typedef struct ParticleSystem {
    Particle items[MAX_PARTICLES];
} ParticleSystem;

void Particle_InitAll(ParticleSystem *ps);
void Particle_SpawnIcy(ParticleSystem *ps, Vector2 pos, float spreadX);
void Particle_UpdateAll(ParticleSystem *ps, float dt);
void Particle_DrawAll(const ParticleSystem *ps);

#endif /* PARTICLE_H */