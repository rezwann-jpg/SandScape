#include "simulation.h"

int brushSize = 1;

enum PARTICLE_TYPE currentParticleType = EMPTY;

Particle grid[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE];

const int width = GRID_WIDTH / CELL_SIZE;
const int height = GRID_HEIGHT / CELL_SIZE;

bool inBounds(int x, int y) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

void intiGrid() {
    for (int y = 0; y < GRID_HEIGHT / CELL_SIZE; y++) {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++) {
            grid[y][x] = (Particle){ .type = EMPTY };
        }
    }
}

Particle createParticle(int type) {
    switch (type) {
        case PARTICLE_SAND: {
            int idx = rand() % SAND_PALETTE_SIZE;
            return (Particle){ .type = PARTICLE_SAND, .color = sandPalette[idx] };
        }
        case PARTICLE_WATER: {
            int idx = rand() % WATER_PALETTE_SIZE;
            return (Particle){ .type = PARTICLE_WATER, .color = waterPalette[idx] };
        }
        case PARTICLE_SMOKE:
            return (Particle){ .type = PARTICLE_SMOKE, .lifespan = SMOKE_LIFESPAN };
        case PARTICLE_FIRE:
            return (Particle){ .type = PARTICLE_FIRE, .lifespan = FIRE_LIFESPAN };
        case PARTICLE_STONE: {
            int idx = rand() % STONE_PALETTE_SIZE;
            return (Particle){ .type = PARTICLE_STONE, .color = stonePalette[idx] };
        }
        case PARTICLE_OIL:
            return (Particle){ .type = PARTICLE_OIL };
        case PARTICLE_LAVA: {
            int idx = rand() % LAVA_PALETTE_SIZE;
            return (Particle){ .type = PARTICLE_LAVA, .color = lavaPalette[idx] };
        }
        case PARTICLE_CLEAR:
            return (Particle){ .type = EMPTY };
        case PARTICLE_WET_SAND:
            return (Particle){ .type = PARTICLE_WET_SAND };
        case PARTICLE_OBSIDIAN:
            return (Particle){ .type = PARTICLE_OBSIDIAN };
        case PARTICLE_STEAM: {
            int idx = rand() % STEAM_PALETTE_SIZE;
            return (Particle){ .type = PARTICLE_STEAM, .lifespan = STEAM_LIFESPAN, .color = steamPalette[idx] };
        }

        default:
            return (Particle){ .type = EMPTY };
    }
}

void addParticle(int x, int y) {
    if (x >= 0 && x < GRID_WIDTH / CELL_SIZE && y >= 0 && y < GRID_HEIGHT / CELL_SIZE) {
        for (int dy = -brushSize; dy <= brushSize; dy++) {
            for (int dx = -brushSize; dx <= brushSize; dx++) {
                if (dx * dx + dy * dy <= brushSize * brushSize) {
                    int targetX = x + dx;
                    int targetY = y + dy;
                    if (targetX >= 0 && targetX < GRID_WIDTH / CELL_SIZE && targetY >= 0 && targetY < GRID_HEIGHT / CELL_SIZE) {
                        grid[targetY][targetX] = createParticle(currentParticleType);
                    }
                }
            }
        }
    }
}

int smokeTimer = 0;
int fireTimer = 0;

void updateParticles() {
    Particle updatedGrid[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE];
    bool visited[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE] = {false};
    memcpy(updatedGrid, grid, sizeof(updatedGrid));

    for (int y = GRID_HEIGHT / CELL_SIZE - 1; y >= 0; y--) {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++) {
            if (visited[y][x]) continue;

            Particle *cell = &grid[y][x];
            if (cell->type == EMPTY) continue;

            switch (cell->type) {
                case PARTICLE_SAND:
                    if (inBounds(x, y) && (grid[y - 1][x].type == PARTICLE_WET_SAND || 
                        grid[y + 1][x].type == PARTICLE_WET_SAND || 
                        grid[y][x - 1].type == PARTICLE_WET_SAND || 
                        grid[y][x + 1].type == PARTICLE_WET_SAND)) {
                        if ((((float)rand() / (float)(RAND_MAX)) * 1.0f) < WET_SAND_PROBABILITY) {
                            updatedGrid[y][x] = createParticle(PARTICLE_WET_SAND);
                        }
                    }

                    if (inBounds(x, y + 1) && grid[y + 1][x].type == EMPTY) {
                        updatedGrid[y][x].type = EMPTY;
                        updatedGrid[y + 1][x] = *cell;
                    }
                    else if (inBounds(x, y + 1)) {
                        bool canLeft = inBounds(x - 1, y + 1) && grid[y + 1][x - 1].type == EMPTY;
                        bool canRight = inBounds(x + 1, y + 1) && grid[y + 1][x + 1].type == EMPTY;
                        
                        if (canLeft && canRight) {
                            int dir = rand() % 2;
                            updatedGrid[y][x].type = EMPTY;
                            if (dir == 0) {
                                updatedGrid[y + 1][x - 1] = *cell;
                            } else {
                                updatedGrid[y + 1][x + 1] = *cell;
                            }
                        }
                        else if (canLeft) {
                            updatedGrid[y][x].type = EMPTY;
                            updatedGrid[y + 1][x - 1] = *cell;
                        }
                        else if (canRight) {
                            updatedGrid[y][x].type = EMPTY;
                            updatedGrid[y + 1][x + 1] = *cell;
                        }
                        else if (grid[y + 1][x].type == PARTICLE_WATER) {
                            updatedGrid[y][x] = createParticle(PARTICLE_WATER);
                            updatedGrid[y + 1][x] = createParticle(PARTICLE_WET_SAND);
                        }
                        else if (grid[y - 1][x].type == PARTICLE_WATER) {
                            updatedGrid[y][x] = createParticle(PARTICLE_WET_SAND);
                        }
                        
                    }
                    break;

                case PARTICLE_WATER:
                    if (!visited[y][x]) {
                        visited[y][x] = true;
                        if (inBounds(x, y + 1) && !visited[y + 1][x] && grid[y + 1][x].type == EMPTY) {
                            updatedGrid[y][x].type = EMPTY;
                            updatedGrid[y + 1][x] = *cell;
                            visited[y + 1][x] = true;
                        }
                        else if (inBounds(x, y + 1)) {
                            bool canLeft = inBounds(x - 1, y + 1) && !visited[y + 1][x - 1] && grid[y + 1][x - 1].type == EMPTY;
                            bool canRight = inBounds(x + 1, y - 1) && !visited[y + 1][x + 1] && grid[y + 1][x + 1].type == EMPTY;
                            
                            if (canLeft && canRight) {
                                int dir = rand() % 2;
                                updatedGrid[y][x].type = EMPTY;
                                if (dir == 0) {
                                    updatedGrid[y + 1][x - 1] = *cell;
                                    visited[y + 1][x - 1] = true;
                                } else {
                                    updatedGrid[y + 1][x + 1] = *cell;
                                    visited[y + 1][x + 1] = true;
                                }
                            }
                            else if (canLeft) {
                                updatedGrid[y][x].type = EMPTY;
                                updatedGrid[y + 1][x - 1] = *cell;
                                visited[y + 1][x - 1] = true;
                            }
                            else if (canRight) {
                                updatedGrid[y][x].type = EMPTY;
                                updatedGrid[y + 1][x + 1] = *cell;
                                visited[y + 1][x + 1] = true;
                            }
                            else {
                                bool canMoveLeft = inBounds(x - 1, y) && !visited[y][x - 1] && grid[y][x - 1].type == EMPTY;
                                bool canMoveRight = inBounds(x + 1, y) && !visited[y][x + 1] && grid[y][x + 1].type == EMPTY;
                                
                                int dir = rand() % 2;
                                if (dir == 0 && canMoveLeft) {
                                    updatedGrid[y][x].type = EMPTY;
                                    updatedGrid[y][x - 1] = *cell;
                                    visited[y][x - 1] = true;
                                }
                                else if (dir == 1 && canMoveRight) {
                                    updatedGrid[y][x].type = EMPTY;
                                    updatedGrid[y][x + 1] = *cell;
                                    visited[y][x + 1] = true;
                                }
                            }
                        }

                        if (inBounds(x, y - 1) && (grid[y - 1][x].type == PARTICLE_LAVA || grid[y - 1][x].type == PARTICLE_FIRE)) {
                            updatedGrid[y][x] = createParticle(PARTICLE_STEAM);
                        }
                        else if (inBounds(x, y + 1) && (grid[y + 1][x].type == PARTICLE_LAVA || grid[y + 1][x].type == PARTICLE_FIRE)) {
                            updatedGrid[y][x] = createParticle(PARTICLE_STEAM);
                        }
                        else if (inBounds(x - 1, y) && (grid[y][x - 1].type == PARTICLE_LAVA || grid[y][x - 1].type == PARTICLE_FIRE)) {
                            updatedGrid[y][x] = createParticle(PARTICLE_STEAM);
                        }
                        else if (inBounds(x + 1, y) && (grid[y][x + 1].type == PARTICLE_LAVA || grid[y][x + 1].type == PARTICLE_FIRE)) {
                            updatedGrid[y][x] = createParticle(PARTICLE_STEAM);
                        }
                    }
                    break;

                case PARTICLE_WET_SAND:
                    if ((inBounds(x, y + 1) && grid[y + 1][x].type == EMPTY) || (inBounds(x, y + 1) && grid[y + 1][x].type == PARTICLE_WATER)) {
                        Particle temp = updatedGrid[y + 1][x];
                        updatedGrid[y + 1][x] = updatedGrid[y][x];
                        updatedGrid[y][x] = temp;
                    } 
                    else if (inBounds(x, y + 1)) {
                        bool canLeft = inBounds(x - 1, y + 1) && (grid[y + 1][x - 1].type == EMPTY || grid[y + 1][x - 1].type == PARTICLE_WATER);
                        bool canRight = inBounds(x + 1, y + 1) && (grid[y + 1][x + 1].type == EMPTY || grid[y + 1][x + 1].type == PARTICLE_WATER);

                        if (canLeft && canRight) {
                            int dir = rand() % 2;
                            if (dir == 0) {
                                Particle temp = updatedGrid[y + 1][x - 1];
                                updatedGrid[y + 1][x - 1] = updatedGrid[y][x];
                                updatedGrid[y][x] = temp;
                            } else {
                                Particle temp = updatedGrid[y + 1][x + 1];
                                updatedGrid[y + 1][x + 1] = updatedGrid[y][x];
                                updatedGrid[y][x] = temp;
                            }
                        } else if (canLeft) {
                            Particle temp = updatedGrid[y + 1][x - 1];
                            updatedGrid[y + 1][x - 1] = updatedGrid[y][x];
                            updatedGrid[y][x] = temp;
                        } else if (canRight) {
                            Particle temp = updatedGrid[y + 1][x + 1];
                            updatedGrid[y + 1][x + 1] = updatedGrid[y][x];
                            updatedGrid[y][x] = temp;
                        }
                    }
                    break;
                
                case PARTICLE_SMOKE:
                    smokeTimer++;
                    if (smokeTimer >= cell->lifespan) {
                        updatedGrid[y][x].type = EMPTY;
                        smokeTimer = 0;
                    }
                    else if (inBounds(x, y - 1) && grid[y - 1][x].type == EMPTY) {
                        updatedGrid[y][x].type = EMPTY;
                        updatedGrid[y - 1][x] = *cell;
                    }
                    else if (inBounds(x - 1, y - 1) && grid[y - 1][x - 1].type == EMPTY)
                    {
                        updatedGrid[y][x].type = EMPTY;
                        updatedGrid[y - 1][x - 1] = *cell;
                    }
                    else if (inBounds(x + 1, y - 1) && grid[y - 1][x + 1].type == EMPTY)
                    {
                        updatedGrid[y][x].type = EMPTY;
                        updatedGrid[y - 1][x + 1] = *cell;
                    }
                    break;

                case PARTICLE_FIRE:
                    fireTimer++;
                    if (fireTimer >= cell->lifespan) {
                        updatedGrid[y][x].type = EMPTY;
                        fireTimer = 0;
                    }
                    else if (inBounds(x, y - 1) && grid[y - 1][x].type == EMPTY) {
                        updatedGrid[y][x].type = EMPTY;
                        updatedGrid[y - 1][x] = *cell;
                    }
                    else if (inBounds(x - 1, y - 1) && grid[y - 1][x - 1].type == EMPTY)
                    {
                        updatedGrid[y][x].type = EMPTY;
                        updatedGrid[y - 1][x - 1] = *cell;
                    }
                    else if (inBounds(x + 1, y - 1) && grid[y - 1][x + 1].type == EMPTY)
                    {
                        updatedGrid[y][x].type = EMPTY;
                        updatedGrid[y - 1][x + 1] = *cell;
                    }
                    break;

                case PARTICLE_OIL:
                    if (!visited[y][x]) {
                        visited[y][x] = true;
                        bool onFire = false;
                        if (inBounds(x, y) && (grid[y - 1][x].type == PARTICLE_FIRE ||
                            grid[y + 1][x].type == PARTICLE_FIRE ||
                            grid[y][x - 1].type == PARTICLE_FIRE ||
                            grid[y][x + 1].type == PARTICLE_FIRE)) {
                            
                            onFire = true;
                        }

                        if (inBounds(x, y) && (grid[y - 1][x].type == PARTICLE_LAVA ||
                            grid[y + 1][x].type == PARTICLE_LAVA ||
                            grid[y][x - 1].type == PARTICLE_LAVA ||
                            grid[y][x + 1].type == PARTICLE_LAVA)) {
                            
                            if ((((float)rand() / (float)(RAND_MAX)) * 1.0f) < OIL_FIRE_PROBANILITY) {
                                onFire = true;
                            }
                        }

                        if (onFire) {
                            updatedGrid[y][x] = createParticle(PARTICLE_FIRE);
                        }
                        else if (inBounds(x, y + 1) && !visited[y + 1][x] && grid[y + 1][x].type == EMPTY) {
                            updatedGrid[y][x].type = EMPTY;
                            updatedGrid[y + 1][x] = *cell;
                            visited[y + 1][x] = true;
                        }
                        else if (inBounds(x, y + 1)) {
                            bool canLeft = inBounds(x - 1, y + 1) && !visited[y + 1][x - 1] && grid[y + 1][x - 1].type == EMPTY;
                            bool canRight = inBounds(x + 1, y - 1) && !visited[y + 1][x + 1] && grid[y + 1][x + 1].type == EMPTY;
                            
                            if (canLeft && canRight) {
                                int dir = rand() % 2;
                                updatedGrid[y][x].type = EMPTY;
                                if (dir == 0) {
                                    updatedGrid[y + 1][x - 1] = *cell;
                                    visited[y + 1][x - 1] = true;
                                } else {
                                    updatedGrid[y + 1][x + 1] = *cell;
                                    visited[y + 1][x + 1] = true;
                                }
                            }
                            else if (canLeft) {
                                updatedGrid[y][x].type = EMPTY;
                                updatedGrid[y + 1][x - 1] = *cell;
                                visited[y + 1][x - 1] = true;
                            }
                            else if (canRight) {
                                updatedGrid[y][x].type = EMPTY;
                                updatedGrid[y + 1][x + 1] = *cell;
                                visited[y + 1][x + 1] = true;
                            }
                            else {
                                bool canMoveLeft = inBounds(x - 1, y) && !visited[y][x - 1] && grid[y][x - 1].type == EMPTY;
                                bool canMoveRight = inBounds(x + 1, y) && !visited[y][x + 1] && grid[y][x + 1].type == EMPTY;
                                
                                int dir = rand() % 2;
                                if (dir == 0 && canMoveLeft) {
                                    updatedGrid[y][x].type = EMPTY;
                                    updatedGrid[y][x - 1] = *cell;
                                    visited[y][x - 1] = true;
                                }
                                else if (dir == 1 && canMoveRight) {
                                    updatedGrid[y][x].type = EMPTY;
                                    updatedGrid[y][x + 1] = *cell;
                                    visited[y][x + 1] = true;
                                }
                            }
                        }
                    }
                    break;

                case PARTICLE_LAVA:
                    if (!visited[y][x]) {
                        visited[y][x] = true;

                        if (inBounds(x, y) && (grid[y - 1][x].type == PARTICLE_WATER ||
                            grid[y][x - 1].type == PARTICLE_WATER ||
                            grid[y][x + 1].type == PARTICLE_WATER)) {

                            updatedGrid[y][x] = createParticle(PARTICLE_OBSIDIAN);        
                        }

                        else if (inBounds(x, y + 1) && !visited[y + 1][x] && grid[y + 1][x].type == EMPTY) {
                            updatedGrid[y][x].type = EMPTY;
                            updatedGrid[y + 1][x] = *cell;
                            visited[y + 1][x] = true;
                        }
                        else if (inBounds(x, y + 1)) {
                            bool canLeft = inBounds(x - 1, y + 1) && !visited[y + 1][x - 1] && grid[y + 1][x - 1].type == EMPTY;
                            bool canRight = inBounds(x + 1, y - 1) && !visited[y + 1][x + 1] && grid[y + 1][x + 1].type == EMPTY;
                            
                            if (canLeft && canRight) {
                                int dir = rand() % 2;
                                updatedGrid[y][x].type = EMPTY;
                                if (dir == 0) {
                                    updatedGrid[y + 1][x - 1] = *cell;
                                    visited[y + 1][x - 1] = true;
                                } else {
                                    updatedGrid[y + 1][x + 1] = *cell;
                                    visited[y + 1][x + 1] = true;
                                }
                            }
                            else if (canLeft) {
                                updatedGrid[y][x].type = EMPTY;
                                updatedGrid[y + 1][x - 1] = *cell;
                                visited[y + 1][x - 1] = true;
                            }
                            else if (canRight) {
                                updatedGrid[y][x].type = EMPTY;
                                updatedGrid[y + 1][x + 1] = *cell;
                                visited[y + 1][x + 1] = true;
                            }
                            else {
                                bool canMoveLeft = inBounds(x - 1, y) && !visited[y][x - 1] && grid[y][x - 1].type == EMPTY;
                                bool canMoveRight = inBounds(x + 1, y) && !visited[y][x + 1] && grid[y][x + 1].type == EMPTY;
                                
                                int dir = rand() % 2;
                                if (dir == 0 && canMoveLeft) {
                                    updatedGrid[y][x].type = EMPTY;
                                    updatedGrid[y][x - 1] = *cell;
                                    visited[y][x - 1] = true;
                                }
                                else if (dir == 1 && canMoveRight) {
                                    updatedGrid[y][x].type = EMPTY;
                                    updatedGrid[y][x + 1] = *cell;
                                    visited[y][x + 1] = true;
                                }
                            }
                        }
                    }
                    break;

                case PARTICLE_STEAM:
                    if (!visited[y][x]) {
                        visited[y][x] = true;

                        updatedGrid[y][x].lifespan--;
                        if (updatedGrid[y][x].lifespan <= 0) {
                            updatedGrid[y][x].type = EMPTY;
                        } else {
                            if (inBounds(x, y - 1) && !visited[y - 1][x] && grid[y - 1][x].type == EMPTY) {
                                updatedGrid[y][x].type = EMPTY;
                                updatedGrid[y - 1][x] = *cell;
                                visited[y - 1][x] = true;
                            }
                        }
                    }
                    break;                
            }
        }
    }
    
    memcpy(grid, updatedGrid, sizeof(grid));
}