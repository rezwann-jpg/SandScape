#ifndef GAME_H_
#define GAME_H_

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "simulation.h"

#define GAME_TITLE "SandScape"

#define WINDOW_HEIGHT (GRID_HEIGHT + BUTTON_AREA_HEIGHT)
#define WINDOW_WIDTH GRID_WIDTH

#define BUTTON_AREA_HEIGHT 70

#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 50
#define BUTTON_SPACING 10

#define NUM_PARTICLES 9

extern const char *buttonLabels[];
extern char fpsText[];

void intializeGame(SDL_Window **window, SDL_Renderer **renderer, TTF_Font **font);
void handleBrushButtonClick(int mouseX, int mouseY);
void handleButtonClick(int mouseX, int mouseY);
void handleEvents(SDL_Event *event, bool *running, bool *isDragging, int *prevMouseX, int *prevMouseY);
void renderButtons(SDL_Renderer *renderer, TTF_Font *font);
void renderGameStats(SDL_Renderer *renderer, TTF_Font *font);
void render(SDL_Renderer *renderer, TTF_Font *font);
void update();
void cleanUp(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font);

#endif /* GAME_H_ */