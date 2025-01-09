#ifndef SIMULATION_H_
#define SIMULATION_H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "color_palettes.h"

#define GRID_WIDTH 1280
#define GRID_HEIGHT 720
#define CELL_SIZE 4

#define SMOKE_LIFESPAN 60
#define FIRE_LIFESPAN 10
#define STEAM_LIFESPAN 100

#define WET_SAND_PROBABILITY 0.002f
#define OIL_FIRE_PROBANILITY 0.05f

enum PARTICLE_TYPE {
    EMPTY,
    PARTICLE_SAND,
    PARTICLE_WATER,
    PARTICLE_SMOKE,
    PARTICLE_FIRE,
    PARTICLE_STONE,
    PARTICLE_OIL,
    PARTICLE_LAVA,
    PARTICLE_CLEAR,
    PARTICLE_OBSIDIAN,
    PARTICLE_WET_SAND,
    PARTICLE_STEAM
};

typedef struct {
    enum PARTICLE_TYPE type;
    Color color;
    int lifespan;
} Particle;

int brushSize;

enum PARTICLE_TYPE currentParticleType;

Particle grid[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE];

void intiGrid();
Particle createParticle(int type);
void addParticle(int x, int y);
void updateParticles();

#endif /* SIMULATION_H_*/