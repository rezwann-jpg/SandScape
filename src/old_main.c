#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION

#include "../include/Nuklear/nuklear.h"
#include "nuklear_sdl_renderer.h"

#define SDL_MAIN_HANDLED
#include "../include/SDL2/SDL.h"
#include "../include/SDL2/SDL_ttf.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define GAME_TITLE "SandScape"

#define FPS 60

#define GRID_WIDTH 1280
#define GRID_HEIGHT 720
#define CELL_SIZE 4

#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 50
#define BUTTON_SPACING 10

#define NUM_PARTICLES 6

const float WET_SAND_PROBABILITY = 0.002;
const float OIL_FIRE_PROBANILITY = 0.05;

enum CELL_TYPE
{
    EMPTY,
};

enum PARTICLE_TYPE
{
    PARTICLE_SAND,
    PARTICLE_WATER,
    PARTICLE_WET_SAND,
    PARTICLE_SMOKE,
    PARTICLE_FIRE,
    PARTICLE_SOLID,
    PARTICLE_CLEAR,
    PARTICLE_OIL,
    PARTICLE_LAVA,
    PARTICLE_OBSIDIAN
};

enum PARTICLE_TYPE currentParticleType = PARTICLE_CLEAR;

int brushSize = 1;

SDL_Rect buttonRects[NUM_PARTICLES];
const char *particleTypeNames[NUM_PARTICLES] = {
    "Sand",
    "Water",
    "Smoke",
    "Oil",
    "Lava",
    "Obsidian"};

int grid[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE];

int smokeTimer = 0;
const int smokeLifeSpan = 60;

int fireTimer = 0;
const int fireLifeSpan = 10;

struct nk_colorf sandColor;

void initGrid()
{
    for (int y = 0; y < GRID_HEIGHT / CELL_SIZE; y++)
    {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++)
        {
            grid[y][x] = EMPTY;
        }
    }

    // don't need it
    // for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++)
    // {
    //     grid[GRID_HEIGHT / CELL_SIZE - 1][x] = PARTICLE_SOLID;
    // }

    // will implement UI buttons in the future
    // int buttonX = BUTTON_SPACING;
    // int buttonY = BUTTON_SPACING;

    // for (int i = 0; i < 3; i++)
    // {
    //     buttonRects[i] = (SDL_Rect){ buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT };
    //     buttonX += BUTTON_WIDTH + BUTTON_SPACING;
    // }
}

void clearGrid()
{
    for (int y = 0; y < GRID_HEIGHT / CELL_SIZE; y++)
    {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++)
        {
            grid[y][x] = EMPTY;
        }
    }

    sandColor.r = 0.87059f, sandColor.g = 0.69804f, sandColor.b = 0.43529f, sandColor.a = 1.00000f;
}

void addParticle(int x, int y)
{
    if (x >= 0 && x < GRID_WIDTH / CELL_SIZE && y >= 0 && y < GRID_HEIGHT / CELL_SIZE)
    {
        for (int dy = -brushSize; dy <= brushSize; dy++)
        {
            for (int dx = -brushSize; dx <= brushSize; dx++)
            {
                int targetX = x + dx;
                int targetY = y + dy;
                if (targetX >= 0 && targetX < GRID_WIDTH / CELL_SIZE && targetY >= 0 && targetY < GRID_HEIGHT / CELL_SIZE &&
                    abs(dx) <= brushSize && abs(dy) <= brushSize && currentParticleType == PARTICLE_CLEAR)
                {
                    grid[targetY][targetX] = EMPTY;
                }
                else if (targetX >= 0 && targetX < GRID_WIDTH / CELL_SIZE && targetY >= 0 && targetY < GRID_HEIGHT / CELL_SIZE &&
                         abs(dx) <= brushSize && abs(dy) <= brushSize)
                {
                    grid[targetY][targetX] = currentParticleType + 1;
                }
            }
        }
    }
}

void updateParticles()
{
    int updateGrid[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE];
    int visitedGrid[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE];
    memcpy(updateGrid, grid, sizeof(updateGrid));
    memset(visitedGrid, 0, sizeof(visitedGrid));

    for (int y = GRID_HEIGHT / CELL_SIZE - 2; y >= 0; y--)
    {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++)
        {
            int cellType = grid[y][x];
            if (cellType > 0)
            {
                switch (cellType)
                {
                case PARTICLE_SAND + 1:
                    if ((y > 0 && grid[y - 1][x] == PARTICLE_WET_SAND + 1) ||
                        (y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x] == PARTICLE_WET_SAND + 1) ||
                        (x > 0 && grid[y][x - 1] == PARTICLE_WET_SAND + 1) ||
                        (x < GRID_WIDTH / CELL_SIZE - 1 && grid[y][x + 1] == PARTICLE_WET_SAND + 1))
                    {
                        if ((((float)rand() / (float)(RAND_MAX)) * 1.0f) < WET_SAND_PROBABILITY)
                        {
                            updateGrid[y][x] = PARTICLE_WET_SAND + 1;
                        }
                    }

                    if (y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y + 1][x] = cellType;
                    }
                    else if (x > 0 && y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x - 1] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y + 1][x - 1] = cellType;
                    }
                    else if (x < GRID_WIDTH / CELL_SIZE - 1 && y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x + 1] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y + 1][x + 1] = cellType;
                    }
                    else if (grid[y + 1][x] == PARTICLE_WATER + 1)
                    {
                        updateGrid[y][x] = PARTICLE_WATER + 1;
                        updateGrid[y + 1][x] = PARTICLE_WET_SAND + 1;
                    }
                    else if (grid[y - 1][x] == PARTICLE_WATER + 1)
                    {
                        updateGrid[y][x] = PARTICLE_WET_SAND + 1;
                    }

                    if (x > 0 && x < GRID_WIDTH / CELL_SIZE - 1 && y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x - 1] == EMPTY && grid[y + 1][x + 1] == EMPTY && grid[y + 1][x] != EMPTY)
                    {
                        int directionS = rand() % 2;
                        if (directionS == 0 && x > 0 && y < GRID_HEIGHT / CELL_SIZE - 1)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x - 1] = cellType;
                        }
                        else if (directionS == 1 && x < GRID_WIDTH / CELL_SIZE - 1 && y < GRID_HEIGHT / CELL_SIZE - 1)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x + 1] = cellType;
                        }
                    }
                    break;

                case PARTICLE_WATER + 1:
                    if (!visitedGrid[y][x])
                    {
                        visitedGrid[y][x] = 1;
                        if (y < GRID_HEIGHT / CELL_SIZE - 1 && !visitedGrid[y + 1][x] && grid[y + 1][x] == EMPTY)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x] = cellType;
                            visitedGrid[y + 1][x] = 1;
                        }
                        else if (x > 0 && !visitedGrid[y + 1][x - 1] && grid[y + 1][x - 1] == EMPTY)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x - 1] = cellType;
                            visitedGrid[y + 1][x - 1] = 1;
                        }
                        else if (x < GRID_WIDTH / CELL_SIZE - 1 && !visitedGrid[y + 1][x + 1] && grid[y + 1][x + 1] == EMPTY)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x + 1] = cellType;
                            visitedGrid[y + 1][x + 1] = 1;
                        }
                        else
                        {
                            int directionW = rand() % 2;
                            if (directionW == 0 && x < GRID_WIDTH / CELL_SIZE - 1 && !visitedGrid[y][x + 1] && grid[y][x + 1] == EMPTY)
                            {
                                updateGrid[y][x] = EMPTY;
                                updateGrid[y][x + 1] = cellType;
                                visitedGrid[y][x + 1] = 1;
                                // fprintf(stdout ,"case 1\n");
                            }
                            else if (directionW == 1 && x > 0 && !visitedGrid[y][x - 1] && grid[y][x - 1] == EMPTY)
                            {
                                updateGrid[y][x] = EMPTY;
                                updateGrid[y][x - 1] = cellType;
                                visitedGrid[y][x - 1] = 1;
                                // fprintf(stdout ,"case 2\n");
                            }
                        }
                    }
                    if (y > 0 && grid[y - 1][x] == PARTICLE_FIRE + 1)
                    {
                        updateGrid[y - 1][x] = PARTICLE_SMOKE + 1;
                    }
                    else if (y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x] == PARTICLE_FIRE + 1)
                    {
                        updateGrid[y + 1][x] = PARTICLE_SMOKE + 1;
                    }
                    else if (x > 0 && grid[y][x - 1] == PARTICLE_FIRE + 1)
                    {
                        updateGrid[y][x - 1] = PARTICLE_SMOKE + 1;
                    }
                    else if (x < GRID_WIDTH / CELL_SIZE - 1 && grid[y][x + 1] == PARTICLE_FIRE + 1)
                    {
                        updateGrid[y][x + 1] = PARTICLE_SMOKE + 1;
                    }
                    break;

                case PARTICLE_WET_SAND + 1:
                    if ((y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x] == EMPTY) || (y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x] == PARTICLE_WATER + 1))
                    {
                        int temp = updateGrid[y + 1][x];
                        updateGrid[y + 1][x] = updateGrid[y][x];
                        updateGrid[y][x] = temp;
                    }
                    else if (x > 0 && (grid[y + 1][x - 1] == EMPTY || grid[y + 1][x - 1] == PARTICLE_WATER + 1))
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y + 1][x - 1] = cellType;
                    }
                    else if (x < GRID_WIDTH / CELL_SIZE - 1 && (grid[y + 1][x + 1] == EMPTY || grid[y + 1][x + 1] == PARTICLE_WATER + 1))
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y + 1][x + 1] = cellType;
                    }
                    break;

                case PARTICLE_SMOKE + 1:
                    // if (y > 0 && grid[y - 1][x] == EMPTY)
                    // {
                    //     updateGrid[y][x] = EMPTY;
                    //     updateGrid[y - 1][x] = cellType;
                    // }
                    // else
                    // {
                    //     int random = rand() % 10;
                    //     if (random < 1)
                    //     {
                    //         int directionSmoke = rand() % 4;
                    //         if (directionSmoke == 0 && y > 0 && grid[y - 1][x] == EMPTY)
                    //         {
                    //             updateGrid[y][x] = EMPTY;
                    //             updateGrid[y - 1][x] = cellType;
                    //         }
                    //         else if (directionSmoke == 1 && y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x] == EMPTY)
                    //         {
                    //             updateGrid[y][x] = EMPTY;
                    //             updateGrid[y + 1][x] = cellType;
                    //         }
                    //         else if (directionSmoke == 2 && x > 0 && grid[y][x - 1] == EMPTY)
                    //         {
                    //             updateGrid[y][x] = EMPTY;
                    //             updateGrid[y][x - 1] = cellType;
                    //         }
                    //         else if (directionSmoke == 3 && x < GRID_WIDTH / CELL_SIZE - 1 && grid[y][x + 1] == EMPTY)
                    //         {
                    //             updateGrid[y][x] = EMPTY;
                    //             updateGrid[y][x + 1] = cellType;
                    //         }
                    //     }
                    // }

                    // smokeTimer--;
                    // if (smokeTimer <= 0)
                    // {
                    //     updateGrid[y][x] = EMPTY;
                    // }
                    // break;
                    smokeTimer++;
                    if (smokeTimer >= smokeLifeSpan)
                    {
                        updateGrid[y][x] = EMPTY;
                        smokeTimer = 0;
                    }
                    else if (y > 0 && grid[y - 1][x] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y - 1][x] = cellType;
                    }
                    else if (x > 0 && y > 0 && grid[y - 1][x - 1] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y - 1][x - 1] = cellType;
                    }
                    else if (x < GRID_WIDTH / CELL_SIZE - 1 && y > 0 && grid[y - 1][x + 1] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y - 1][x + 1] = cellType;
                    }
                    break;

                case PARTICLE_FIRE + 1:
                    // if (y > 0 && grid[y - 1][x] == EMPTY &&)
                    // {
                    //     updateGrid[y][x] = EMPTY;
                    //     updateGrid[y - 1][x] = cellType;
                    // }
                    // else
                    // {
                    //     if (!visitedGrid[y][x])
                    //     {
                    //         visitedGrid[y][x] = 1;
                    //         int random = rand() % 6;
                    //         if (random == 0 && y < GRID_HEIGHT / CELL_SIZE - 1 && !visitedGrid[y + 1][x] && grid[y + 1][x] == EMPTY)
                    //         {
                    //             int temp = updateGrid[y + 1][x];
                    //             updateGrid[y + 1][x] = updateGrid[y][x];
                    //             updateGrid[y][x] = temp;
                    //             visitedGrid[y + 1][x] = 1;
                    //         }
                    //         else if (random == 1 && x > 0 && !visitedGrid[y + 1][x - 1] && grid[y + 1][x - 1] == EMPTY)
                    //         {
                    //             int temp = updateGrid[y + 1][x - 1];
                    //             updateGrid[y + 1][x - 1] = updateGrid[y][x];
                    //             updateGrid[y][x] = temp;
                    //             visitedGrid[y + 1][x - 1] = 1;
                    //         }
                    //         else if (random == 2 && x < GRID_WIDTH / CELL_SIZE - 1 && !visitedGrid[y + 1][x + 1] && grid[y + 1][x + 1] == EMPTY)
                    //         {
                    //             int temp = updateGrid[y + 1][x + 1];
                    //             updateGrid[y + 1][x + 1] = updateGrid[y][x];
                    //             updateGrid[y][x] = temp;
                    //             visitedGrid[y + 1][x + 1] = 1;
                    //         }
                    //         else if (random == 3 && y > 0 && x > 0 && !visitedGrid[y - 1][x - 1] && grid[y - 1][x - 1] == EMPTY)
                    //         {
                    //             int temp = updateGrid[y - 1][x - 1];
                    //             updateGrid[y - 1][x - 1] = updateGrid[y][x];
                    //             updateGrid[y][x] = temp;
                    //             visitedGrid[y - 1][x - 1] = 1;
                    //         }
                    //         else if (random == 4 && y > 0 && x < GRID_WIDTH / CELL_SIZE - 1 && !visitedGrid[y - 1][x + 1] && grid[y - 1][x + 1] == EMPTY)
                    //         {
                    //             int temp = updateGrid[y - 1][x + 1];
                    //             updateGrid[y - 1][x + 1] = updateGrid[y][x];
                    //             updateGrid[y][x] = temp;
                    //             visitedGrid[y - 1][x + 1] = 1;
                    //         }
                    //         else if (random == 5 && y > 0 && !visitedGrid[y - 1][x] && grid[y - 1][x] == EMPTY)
                    //         {
                    //             int temp = updateGrid[y - 1][x];
                    //             updateGrid[y - 1][x] = updateGrid[y][x];
                    //             updateGrid[y][x] = temp;
                    //             visitedGrid[y - 1][x] = 1;
                    //         }
                    //     }
                    // }

                    // fireTimer -= 2;
                    // if (fireTimer <= 0)
                    // {
                    //     updateGrid[y][x] = EMPTY;
                    // }
                    // break;
                    fireTimer++;
                    if (fireTimer >= fireLifeSpan)
                    {
                        updateGrid[y][x] = EMPTY;
                        fireTimer = 0;
                    }
                    else if (y > 0 && grid[y - 1][x] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y - 1][x] = cellType;
                    }
                    else if (x > 0 && y > 0 && grid[y - 1][x - 1] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y - 1][x - 1] = cellType;
                    }
                    else if (x < GRID_WIDTH / CELL_SIZE - 1 && y > 0 && grid[y - 1][x + 1] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y - 1][x + 1] = cellType;
                    }
                    break;

                case PARTICLE_OIL + 1:
                    if (!visitedGrid[y][x])
                    {
                        visitedGrid[y][x] = 1;
                        int onFire = 0;
                        if ((y > 0 && grid[y - 1][x] == PARTICLE_FIRE + 1) ||
                            (y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x] == PARTICLE_FIRE + 1) ||
                            (x > 0 && grid[y][x - 1] == PARTICLE_FIRE + 1) ||
                            (x < GRID_WIDTH / CELL_SIZE - 1 && grid[y][x + 1] == PARTICLE_FIRE + 1))
                        {
                            onFire = 1;
                        }

                        if ((y > 0 && grid[y - 1][x] == PARTICLE_LAVA + 1) ||
                            (y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x] == PARTICLE_LAVA + 1) ||
                            (x > 0 && grid[y][x - 1] == PARTICLE_LAVA + 1) ||
                            (x < GRID_WIDTH / CELL_SIZE - 1 && grid[y][x + 1] == PARTICLE_LAVA + 1))
                        {
                            if ((((float)rand() / (float)(RAND_MAX)) * 1.0f) < OIL_FIRE_PROBANILITY)
                            {
                                onFire = 1;
                            }
                        }

                        if (onFire)
                        {
                            updateGrid[y][x] = PARTICLE_FIRE + 1;
                        }
                        else if (y < GRID_HEIGHT / CELL_SIZE - 1 && !visitedGrid[y + 1][x] && grid[y + 1][x] == EMPTY)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x] = cellType;
                            visitedGrid[y + 1][x] = 1;
                        }
                        else if (x > 0 && !visitedGrid[y + 1][x - 1] && grid[y + 1][x - 1] == EMPTY)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x - 1] = cellType;
                            visitedGrid[y + 1][x - 1] = 1;
                        }
                        else if (x < GRID_WIDTH / CELL_SIZE - 1 && !visitedGrid[y + 1][x + 1] && grid[y + 1][x + 1] == EMPTY)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x + 1] = cellType;
                            visitedGrid[y + 1][x + 1] = 1;
                        }
                        else
                        {
                            int directionO = rand() % 2;
                            if (directionO == 0 && x < GRID_WIDTH / CELL_SIZE - 1 && !visitedGrid[y][x + 1] && grid[y][x + 1] == EMPTY)
                            {
                                updateGrid[y][x] = EMPTY;
                                updateGrid[y][x + 1] = cellType;
                                visitedGrid[y][x + 1] = 1;
                            }
                            else if (directionO == 1 && x > 0 && !visitedGrid[y][x - 1] && grid[y][x - 1] == EMPTY)
                            {
                                updateGrid[y][x] = EMPTY;
                                updateGrid[y][x - 1] = cellType;
                                visitedGrid[y][x - 1] = 1;
                            }
                        }
                    }
                    break;

                case PARTICLE_LAVA + 1:
                    if (!visitedGrid[y][x])
                    {
                        visitedGrid[y][x] = 1;
                        int contactWithWater = 0;

                        if ((y > 0 && grid[y - 1][x] == PARTICLE_WATER + 1) ||
                            (x > 0 && grid[y][x - 1] == PARTICLE_WATER + 1) ||
                            (x < GRID_WIDTH / CELL_SIZE - 1 && grid[y][x + 1] == PARTICLE_WATER + 1))
                        {
                            updateGrid[y][x] = PARTICLE_OBSIDIAN + 1;
                        }
                        
                        if (y < GRID_HEIGHT / CELL_SIZE - 1 && grid[y + 1][x] == PARTICLE_WATER + 1)
                        {
                            contactWithWater = 1;
                        }
                        if (contactWithWater)
                        {
                            int temp = updateGrid[y + 1][x];
                            updateGrid[y + 1][x] = updateGrid[y][x];
                            updateGrid[y][x] = temp;
                            updateGrid[y + 1][x] = PARTICLE_SOLID + 1;
                        }
                        else if (y < GRID_HEIGHT / CELL_SIZE - 1 && !visitedGrid[y + 1][x] && grid[y + 1][x] == EMPTY)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x] = cellType;
                            visitedGrid[y + 1][x] = 1;
                        }
                        else if (x > 0 && !visitedGrid[y + 1][x - 1] && grid[y + 1][x - 1] == EMPTY)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x - 1] = cellType;
                            visitedGrid[y + 1][x - 1] = 1;
                        }
                        else if (x < GRID_WIDTH / CELL_SIZE - 1 && !visitedGrid[y + 1][x + 1] && grid[y + 1][x + 1] == EMPTY)
                        {
                            updateGrid[y][x] = EMPTY;
                            updateGrid[y + 1][x + 1] = cellType;
                            visitedGrid[y + 1][x + 1] = 1;
                        }
                        else
                        {
                            int directionL = rand() % 2;
                            if (directionL == 0 && x < GRID_WIDTH / CELL_SIZE - 1 && !visitedGrid[y][x + 1] && grid[y][x + 1] == EMPTY)
                            {
                                updateGrid[y][x] = EMPTY;
                                updateGrid[y][x + 1] = cellType;
                                visitedGrid[y][x + 1] = 1;
                            }
                            else if (directionL == 1 && x > 0 && !visitedGrid[y][x - 1] && grid[y][x - 1] == EMPTY)
                            {
                                updateGrid[y][x] = EMPTY;
                                updateGrid[y][x - 1] = cellType;
                                visitedGrid[y][x - 1] = 1;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    // smokeTimer = smokeLifeSpan;
    // fireTimer = fireLifeSpan;

    memcpy(grid, updateGrid, sizeof(grid));
}

void renderGame(SDL_Renderer *renderer, TTF_Font *font)
{
    // SDL_SetRenderDrawColor(renderer, 35, 31, 32, 255);
    SDL_SetRenderDrawColor(renderer, 243, 230, 230, 255);
    SDL_RenderClear(renderer);

    // buttons rendering, will do it later
    // SDL_SetRenderDrawColor(renderer, 0, 242, 255, 255);
    // for (int i = 0; i < NUM_PARTICLES; i++)
    // {
    //     SDL_RenderFillRect(renderer, &buttonRects[i]);
    //     SDL_Color textColor = {255, 255, 0, 255};
    //     SDL_Surface *textSurface = TTF_RenderText_Solid(font, particleTypeNames[i], textColor);
    //     if (textSurface != NULL)
    //     {
    //         SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    //         if (textTexture != NULL)
    //         {
    //             SDL_Rect textRect = {
    //                 buttonRects[i].x + (BUTTON_WIDTH - textSurface->w) / 2,
    //                 buttonRects[i].y + (BUTTON_HEIGHT - textSurface->h) / 2,
    //                 textSurface->w,
    //                 textSurface->h};
    //             SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    //             SDL_DestroyTexture(textTexture);
    //         }
    //         SDL_FreeSurface(textSurface);
    //     }
    //     else
    //     {
    //         fprintf(stderr, "Error rendering text: %s\n", TTF_GetError());
    //     }
    // }

    for (int y = 0; y < GRID_HEIGHT / CELL_SIZE; y++)
    {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++)
        {
            SDL_Rect rect = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            switch (grid[y][x])
            {
            case PARTICLE_SAND + 1:
                SDL_SetRenderDrawColor(renderer, sandColor.r * 255, sandColor.g * 255, sandColor.b * 255, sandColor.a * 255);
                break;

            case PARTICLE_WATER + 1:
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                break;

            case PARTICLE_WET_SAND + 1:
                SDL_SetRenderDrawColor(renderer, 174, 143, 96, 255);
                break;

            case PARTICLE_SMOKE + 1:
                SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
                break;

            case PARTICLE_FIRE + 1:
                SDL_SetRenderDrawColor(renderer, 255, 126, 0, 255);
                break;

            case PARTICLE_SOLID + 1:
                SDL_SetRenderDrawColor(renderer, 136, 140, 141, 255);
                break;

            case PARTICLE_CLEAR + 1:
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                break;

            case PARTICLE_OIL + 1:
                SDL_SetRenderDrawColor(renderer, 54, 17, 2, 255);
                break;

            case PARTICLE_LAVA + 1:
                SDL_SetRenderDrawColor(renderer, 255, 69, 0, 255);
                break;

            case PARTICLE_OBSIDIAN + 1:
                SDL_SetRenderDrawColor(renderer, 6, 3, 19, 255);
                break;

            default:
                continue;
            }

            SDL_RenderFillRect(renderer, &rect);
        }
    }

    nk_sdl_render(NK_ANTI_ALIASING_ON);

    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    struct nk_context *ctx;
    float font_scale = 1;

    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
    }

    if (TTF_Init() < 0)
    {
        fprintf(stderr, "Font initializaation failed: %s\n", TTF_GetError());
    }

    TTF_Font *font = TTF_OpenFont("../assets/MonospaceTypewriter.ttf", 24);
    if (font == NULL)
    {
        fprintf(stderr, "Font creation failed: %s\n", TTF_GetError());
    }

    SDL_Window *window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GRID_WIDTH, GRID_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
    }

#if 0
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
#endif

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == NULL)
    {
        fprintf(stderr, "Renderer could not be created: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;
    }

    {
        int render_w, render_h;
        int window_w, window_h;
        float scale_x, scale_y;
        SDL_GetRendererOutputSize(renderer, &render_w, &render_h);
        SDL_GetWindowSize(window, &window_w, &window_h);
        scale_x = (float)(render_w) / (float)(window_w);
        scale_y = (float)(render_h) / (float)(window_h);
        SDL_RenderSetScale(renderer, scale_x, scale_y);
        font_scale = scale_y;
    }

    ctx = nk_sdl_init(window, renderer);

    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        nk_sdl_font_stash_end();

        font->handle.height /= font_scale;
        nk_style_set_font(ctx, &font->handle);
    }

    srand(time(NULL));

    initGrid();

    SDL_Event event;
    bool running = true;

    bool isDragging = false;
    int prevMouseX = -1, prevMouseY = -1;

    const int desiredDelta = 1000 / FPS;

    sandColor.r = 0.87059f, sandColor.g = 0.69804f, sandColor.b = 0.43529f, sandColor.a = 1.00000f;

    while (running)
    {
        Uint32 startLoop = SDL_GetTicks();

        nk_input_begin(ctx);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mousePosX, mousePosY;
                SDL_GetMouseState(&mousePosX, &mousePosY);
                if (mousePosY < GRID_HEIGHT)
                {
                    isDragging = true;
                    prevMouseX = mousePosX / CELL_SIZE;
                    prevMouseY = mousePosY / CELL_SIZE;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                isDragging = false;
                prevMouseX = -1;
                prevMouseY = -1;
            }
            else if (event.type == SDL_MOUSEMOTION && isDragging)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (y < GRID_HEIGHT)
                {
                    int currX = x / CELL_SIZE;
                    int currY = y / CELL_SIZE;
                    if (currX != prevMouseX || currY != prevMouseY)
                    {
                        addParticle(currX, currY);
                        prevMouseX = currX;
                        prevMouseY = currY;
                    }
                }
            }
            // else if (event.type == SDL_KEYDOWN)
            // {
            //     if (event.key.keysym.sym == SDLK_UP)
            //     {
            //         fprintf(stdout, "Sand Selected\n");
            //         currentParticleType = PARTICLE_SAND;
            //     }
            //     else if (event.key.keysym.sym == SDLK_DOWN)
            //     {
            //         fprintf(stdout, "Water Selected\n");
            //         currentParticleType = PARTICLE_WATER;
            //     }
            //     else if (event.key.keysym.sym == SDLK_RIGHT)
            //     {
            //         fprintf(stdout, "Smoke Selected\n");
            //         currentParticleType = PARTICLE_SMOKE;
            //     }
            //     else if (event.key.keysym.sym == SDLK_LEFT)
            //     {
            //         fprintf(stdout, "Erase Selected\n");
            //         currentParticleType = PARTICLE_CLEAR;
            //     }
            //     else if (event.key.keysym.sym == SDLK_o)
            //     {
            //         fprintf(stdout, "Stone Selected\n");
            //         currentParticleType = PARTICLE_SOLID;
            //     }
            //     else if (event.key.keysym.sym == SDLK_l)
            //     {
            //         fprintf(stdout, "Fire Selected\n");
            //         currentParticleType = PARTICLE_FIRE;
            //     }
            //     else if (event.key.keysym.sym == SDLK_r)
            //     {
            //         clearGrid();
            //         fprintf(stdout, "Game Cleared\n");
            //     }
            //     else if (event.key.keysym.sym == SDLK_EQUALS)
            //     {
            //         brushSize++;
            //         fprintf(stdout, "Brush Size : %d\n", brushSize);
            //     }
            //     else if (event.key.keysym.sym == SDLK_MINUS)
            //     {
            //         brushSize = (brushSize > 1) ? brushSize - 1 : 1;
            //         fprintf(stdout, "Brush Size : %d\n", brushSize);
            //     }
            // }
            nk_sdl_handle_event(&event);
        }

        nk_sdl_handle_grab();
        nk_input_end(ctx);

        if (nk_begin(ctx, "Controls", nk_rect(0, 0, 210, 350), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_BACKGROUND))
        {
            nk_layout_row_static(ctx, 30, 80, 1);
            nk_label(ctx, "Particles:", NK_TEXT_ALIGN_LEFT);

            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_button_label(ctx, "Sand"))
            {
                fprintf(stdout, "Sand Selected\n");
                currentParticleType = PARTICLE_SAND;
            }
            if (nk_button_label(ctx, "Water"))
            {
                fprintf(stdout, "Water Selected\n");
                currentParticleType = PARTICLE_WATER;
            }
            if (nk_button_label(ctx, "Smoke"))
            {
                fprintf(stdout, "Smoke Selected\n");
                currentParticleType = PARTICLE_SMOKE;
            }
            if (nk_button_label(ctx, "Stone"))
            {
                fprintf(stdout, "Stone Selected\n");
                currentParticleType = PARTICLE_SOLID;
            }
            if (nk_button_label(ctx, "Erase"))
            {
                fprintf(stdout, "Erase Selected\n");
                currentParticleType = PARTICLE_CLEAR;
            }
            if (nk_button_label(ctx, "Fire"))
            {
                fprintf(stdout, "Fire Selected\n");
                currentParticleType = PARTICLE_FIRE;
            }
            if (nk_button_label(ctx, "Oil"))
            {
                fprintf(stdout, "Oil Selected\n");
                currentParticleType = PARTICLE_OIL;
            }
            if (nk_button_label(ctx, "Lava"))
            {
                fprintf(stdout, "Lava Selected\n");
                currentParticleType = PARTICLE_LAVA;
            }

            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "Clear"))
            {
                clearGrid();
                fprintf(stdout, "Game Cleared\n");
            }

            nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
            {
                nk_layout_row_push(ctx, 70);
                nk_label(ctx, "Brush Size:", NK_TEXT_LEFT);
                nk_layout_row_push(ctx, 110);
                nk_slider_int(ctx, 1, &brushSize, 15, 1);
            }
            nk_layout_row_end(ctx);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "Sand Color:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(sandColor), nk_vec2(nk_widget_width(ctx), 400)))
            {
                nk_layout_row_dynamic(ctx, 120, 1);
                sandColor = nk_color_picker(ctx, sandColor, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                sandColor.r = nk_propertyf(ctx, "#R:", 0, sandColor.r, 1.0f, 0.01f, 0.005f);
                sandColor.g = nk_propertyf(ctx, "#G:", 0, sandColor.g, 1.0f, 0.01f, 0.005f);
                sandColor.b = nk_propertyf(ctx, "#B:", 0, sandColor.b, 1.0f, 0.01f, 0.005f);
                sandColor.a = nk_propertyf(ctx, "#A:", 0, sandColor.a, 1.0f, 0.01f, 0.005f);
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);

        updateParticles();
        renderGame(renderer, font);

        int delta = SDL_GetTicks() - startLoop;
        if (delta < desiredDelta)
        {
            SDL_Delay(desiredDelta - delta);
        }
    }

    nk_sdl_shutdown();
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}