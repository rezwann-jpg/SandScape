#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "../include/SDL2/SDL_ttf.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define GAME_TITLE "SandScape"

#define GRID_WIDTH 800
#define GRID_HEIGHT 600
#define CELL_SIZE 4

#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 50
#define BUTTON_SPACING 10

enum CELL_TYPE
{
    EMPTY,
    SAND,
    SOLID
};

enum PARTICLE_TYPE
{
    PARTICLE_SAND,
    PARTICLE_WATER,
    PARTICLE_SMOKE,
    PARTICLE_SOLID,
    NUM_PARTICLE_TYPES
};

enum PARTICLE_TYPE currentParticleType = PARTICLE_SAND;

SDL_Rect buttonRects[3];
const char *particleTypeNames[3] = {
    "Sand",
    "Water",
    "Smoke"};

int grid[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE];

void initGrid()
{
    for (int y = 0; y < GRID_HEIGHT / CELL_SIZE; y++)
    {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++)
        {
            grid[y][x] = EMPTY;
        }
    }

    // for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++)
    // {
    //     grid[GRID_HEIGHT / CELL_SIZE - 1][x] = SOLID;
    // }

    int buttonX = BUTTON_SPACING;
    int buttonY = BUTTON_SPACING;

    for (int i = 0; i < 3; i++)
    {
        buttonRects[i] = (SDL_Rect){ buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT };
        buttonX += BUTTON_WIDTH + BUTTON_SPACING;
    }
}

void addParticle(int x, int y)
{
    if (x >= 0 && x < GRID_WIDTH / CELL_SIZE && y >= 0 && y < GRID_HEIGHT / CELL_SIZE)
    {
        grid[y][x] = currentParticleType + 1;
    }
}

void updateParticles()
{
    int updateGrid[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE];
    memcpy(updateGrid, grid, sizeof(grid));

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
                    if (grid[y + 1][x] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y + 1][x] = cellType;
                    }
                    else if (x > 0 && grid[y + 1][x - 1] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y + 1][x - 1] = cellType;
                    }
                    else if (x < GRID_WIDTH / CELL_SIZE - 1 && grid[y + 1][x + 1] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y + 1][x + 1] = cellType;
                    }
                    break;

                case PARTICLE_WATER + 1:
                    if (grid[y + 1][x] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y + 1][x] = cellType;
                    }
                    else if (x > 0 && grid[y][x - 1] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y][x - 1] = cellType;
                    }
                    else if (x < GRID_WIDTH / CELL_SIZE - 1 && grid[y][x + 1] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y][x + 1] = cellType;
                    }
                    break;

                case PARTICLE_SMOKE + 1:
                    if (y > 0 && grid[y - 1][x] == EMPTY)
                    {
                        updateGrid[y][x] = EMPTY;
                        updateGrid[y - 1][x] = cellType;
                    }
                    break;
                }
            }
        }
    }

    memcpy(grid, updateGrid, sizeof(grid));
}

void renderGame(SDL_Renderer *renderer, TTF_Font *font)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 242, 255, 255);
    for (int i = 0; i < 3; i++)
    {
        SDL_RenderFillRect(renderer, &buttonRects[i]);
        SDL_Color textColor = {255, 255, 0, 255};
        SDL_Surface *textSurface = TTF_RenderText_Solid(font, particleTypeNames[i], textColor);
        if (textSurface != NULL)
        {
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture != NULL)
            {
                SDL_Rect textRect = {
                    buttonRects[i].x + (BUTTON_WIDTH - textSurface->w) / 2,
                    buttonRects[i].y + (BUTTON_HEIGHT - textSurface->h) / 2,
                    textSurface->w,
                    textSurface->h};
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
        else 
        {
            fprintf(stderr, "Error rendering text: %s\n", TTF_GetError());
        }
    }

    for (int y = 0; y < GRID_HEIGHT / CELL_SIZE; y++)
    {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++)
        {
            SDL_Rect rect = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            switch (grid[y][x])
            {
            case PARTICLE_SAND + 1:
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                break;

            case PARTICLE_WATER + 1:
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                break;

            case PARTICLE_SMOKE + 1:
                SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
                break;

            case PARTICLE_SOLID + 1:
                SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
                break;
            default:
                continue;
            }

            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
    }

    if (TTF_Init() < 0)
    {
        fprintf(stderr, "Font initializaation failed: %s\n", TTF_GetError());
    }

    TTF_Font *font = TTF_OpenFont("D:/Dev/SandScape/assets/MonospaceTypewriter.ttf", 24);
    if (font == NULL)
    {
        fprintf(stderr, "Font creation failed: %s\n", TTF_GetError());
    }

    SDL_Window *window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GRID_WIDTH, GRID_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL)
    {
        fprintf(stderr, "Renderer could not be created: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;
    }
    
    initGrid();

    SDL_Event event;
    bool running = true;

    bool isDragging = false;
    int prevMouseX = -1, prevMouseY = -1;

    while (running)
    {
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
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                isDragging = false;
                prevMouseX = -1;
                prevMouseY = -1;
            } else if (event.type == SDL_MOUSEMOTION && isDragging) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (y < GRID_HEIGHT) {
                    int currX = x / CELL_SIZE;
                    int currY = y / CELL_SIZE;
                    if (currX != prevMouseX || currY != prevMouseY) {
                        addParticle(currX, currY);
                        prevMouseX = currX;
                        prevMouseY = currY;
                    }
                }
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_UP) {
                    fprintf(stdout, "Sand Selected\n");
                    currentParticleType = PARTICLE_SAND;
                }
                else if (event.key.keysym.sym == SDLK_DOWN) {
                    fprintf(stdout, "Water Selected\n");
                    currentParticleType = PARTICLE_WATER;
                }
                else if (event.key.keysym.sym == SDLK_RIGHT) {
                    fprintf(stdout, "Smoke Selected\n");
                    currentParticleType = PARTICLE_SMOKE;
                }
            }
        }

        updateParticles();
        renderGame(renderer, font);
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}