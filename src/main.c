#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define GAME_TITLE "SandScape"

#define GRID_WIDTH 800
#define GRID_HEIGHT 600
#define CELL_SIZE 4

enum CELL_TYPE {
    EMPTY,
    SAND,
    SOLID
};

int grid[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE];

void initGrid() {
    for (int y = 0; y < GRID_HEIGHT / CELL_SIZE; y++) {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++) {
            grid[y][x] = EMPTY;
        }
    }

    for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++) {
        grid[GRID_HEIGHT / CELL_SIZE - 1][x] = SOLID;
    }
}

void addSand(int x, int y) {
    if (x >= 0 && x < GRID_WIDTH / CELL_SIZE && y >= 0 && y < GRID_HEIGHT / CELL_SIZE) {
        grid[y][x] = SAND;
    }
}

void updateSand() {
    int updateGrid[GRID_HEIGHT / CELL_SIZE][GRID_WIDTH / CELL_SIZE];
    memcpy(updateGrid, grid, sizeof(grid));

    for (int y = GRID_HEIGHT / CELL_SIZE - 2; y >= 0; y--) {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++) {
            if (grid[y][x] == SAND) {
                if (grid[y + 1][x] == EMPTY) {
                    updateGrid[y][x] = EMPTY;
                    updateGrid[y + 1][x] = SAND;
                }
                else if (x > 0 && grid[y + 1][x - 1] == EMPTY) {
                    updateGrid[y][x] = EMPTY;
                    updateGrid[y + 1][x - 1] = SAND;
                }
                else if (x < GRID_WIDTH / CELL_SIZE - 1 && grid[y + 1][x + 1] == EMPTY) {
                    updateGrid[y][x] = EMPTY;
                    updateGrid[y + 1][x + 1] = SAND;
                }
            }
        }
    }

    memcpy(grid, updateGrid, sizeof(grid));
}

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GRID_WIDTH, GRID_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;
    }


    SDL_Event event;
    bool running = true;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mousePosX, mousePosY;
                SDL_GetMouseState(&mousePosX, &mousePosY);
                addSand(mousePosX / CELL_SIZE, mousePosY / CELL_SIZE);
            }
        }

        updateSand();

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        for (int y = 0; y < GRID_HEIGHT / CELL_SIZE; y++) {
            for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++) {
                SDL_Rect rect = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
                switch (grid[y][x]) {
                case SAND:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                    break;
                
                case SOLID:
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

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}