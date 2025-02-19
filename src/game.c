#include "game.h"

const char *buttonLabels[NUM_PARTICLES] = { "Sand", "Water", "Smoke", "Fire", "Stone", "Oil", "Lava", "Erase", "Clear" };

char fpsText[20] = "FPS: ";

void intializeGame(GameContext* pContext) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "SDL_Initialization error: %s\n", SDL_GetError());
    }

    pContext->window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    pContext->renderer = SDL_CreateRenderer(pContext->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!pContext->window || !pContext->renderer) {
        fprintf(stderr, "SDL Window Creation error: %s\n", SDL_GetError());
    }

    if (TTF_Init() < 0) {
        fprintf(stderr, "Font Initialization error: %s\n", TTF_GetError());
    }

    pContext->font = TTF_OpenFont("assets/MonospaceTypewriter.ttf", 24);
    if (!pContext->font) {
        fprintf(stderr, "Font loading error: %s\n", TTF_GetError());
    }

    srand(time(NULL));

    pContext->isDragging = false;
    pContext->prevMouseX = -1;
    pContext->prevMouseY = -1;

    intiGrid();
}

void handleBrushButtonClick(int mouseX, int mouseY) {
    if (mouseY >= GRID_HEIGHT) {
        int x = 1000, y = GRID_HEIGHT + BUTTON_SPACING;

        SDL_Rect plusButton = { x, y, 50, 50 };
        if (mouseX >= plusButton.x && mouseX <= plusButton.x + 50 &&
            mouseY >= plusButton.y && mouseY <= plusButton.y + 50) {
            
            brushSize++;
            return;
        }

        x += 50 + BUTTON_SPACING;

        SDL_Rect minusButton = { x, y, 50, 50 };
        if (mouseX >= minusButton.x && mouseX <= minusButton.x + 50 &&
            mouseY >= minusButton.y && mouseY <= minusButton.y + 50) {
            
            if (brushSize > 1) {
                brushSize--;
            }
            return;
        }
    }
}

void handleButtonClick(int mouseX, int mouseY) {
    if (mouseY >= GRID_HEIGHT) {
        int x = BUTTON_SPACING, y = GRID_HEIGHT + BUTTON_SPACING;

        for (int i = 0; i < NUM_PARTICLES; i++) {
            SDL_Rect buttonRect = { x, y, BUTTON_WIDTH, BUTTON_HEIGHT };
            if (mouseX >= buttonRect.x && mouseX <= buttonRect.x + BUTTON_WIDTH &&
                mouseY >= buttonRect.y && mouseY <= buttonRect.y + BUTTON_HEIGHT) {
                if (i == NUM_PARTICLES - 1) {
                    fprintf(stdout, "Grid cleared\n");
                    intiGrid();
                    break;
                }
                else {
                    currentParticleType = i + 1;
                    fprintf(stdout, "%s pressed\n", buttonLabels[i]);
                    break;
                }
            }

            x += BUTTON_WIDTH + BUTTON_SPACING;
            if (x + BUTTON_WIDTH > GRID_WIDTH) {
                x = BUTTON_SPACING;
                y += BUTTON_HEIGHT + BUTTON_SPACING;
            }            
        }
    }
}

void handleEvents(GameContext* pContext) {
    while (SDL_PollEvent(&pContext->event)) {
        switch (pContext->event.type) {
            case SDL_QUIT:
                pContext->running = false;
                break;

            case SDL_MOUSEBUTTONDOWN: {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                if (mouseY < GRID_HEIGHT) {
                    pContext->isDragging = true;
                    pContext->prevMouseX = mouseX / CELL_SIZE;
                    pContext->prevMouseY = mouseY / CELL_SIZE;
                }
                else {
                    handleButtonClick(mouseX, mouseY);
                    handleBrushButtonClick(mouseX, mouseY);
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                pContext->isDragging = false;
                pContext->prevMouseX = -1;
                pContext->prevMouseY = -1;
                break;
            }
            case SDL_MOUSEMOTION: {
                if (pContext->isDragging) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    if (y < GRID_HEIGHT) {
                        int currX = x / CELL_SIZE;
                        int currY = y / CELL_SIZE;
                        if (currX != pContext->prevMouseX || currY != pContext->prevMouseY) {
                            addParticle(currX, currY);
                            pContext->prevMouseX = currX;
                            pContext->prevMouseY = currY;
                        }
                    }
                }
                break;
            }
        }
    }
}

void renderButtons(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Rect buttonArea = { 0, GRID_HEIGHT, GRID_WIDTH, BUTTON_AREA_HEIGHT };
    SDL_Color textColor = { 255, 227, 179, 255 };

    SDL_SetRenderDrawColor(renderer, 146, 222, 139, 255);
    SDL_RenderFillRect(renderer, &buttonArea);

    SDL_Rect buttonRects[NUM_PARTICLES];

    int x = BUTTON_SPACING, y = GRID_HEIGHT + BUTTON_SPACING;
    for (int i = 0; i < NUM_PARTICLES; i++) {
        buttonRects[i] = (SDL_Rect){ x, y, BUTTON_WIDTH, BUTTON_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 10, 182, 139, 255);
        SDL_RenderFillRect(renderer, &buttonRects[i]);

        SDL_Surface *textSurface = TTF_RenderText_Solid(font, buttonLabels[i], textColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect textRect = {
            x + (BUTTON_WIDTH - textSurface->w) / 2,
            y + (BUTTON_HEIGHT - textSurface->h) / 2,
            textSurface->w,
            textSurface->h
        };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        x += BUTTON_WIDTH + BUTTON_SPACING;
        if (x + BUTTON_WIDTH > GRID_WIDTH) {
            x = BUTTON_SPACING;
            y += BUTTON_HEIGHT + BUTTON_SPACING;
        }
    }

    SDL_Rect plusButton = { x, y, BUTTON_WIDTH / 2, BUTTON_HEIGHT };
    SDL_SetRenderDrawColor(renderer, 10, 182, 139, 255);
    SDL_RenderFillRect(renderer, &plusButton);

    SDL_Surface *plusSurface = TTF_RenderText_Solid(font, "+", textColor);
    SDL_Texture *plusTexture = SDL_CreateTextureFromSurface(renderer, plusSurface);
    SDL_Rect plusTextRect = {
        plusButton.x + ((BUTTON_WIDTH / 2) - plusSurface->w) / 2,
        plusButton.y + (BUTTON_HEIGHT - plusSurface->h) / 2,
        plusSurface->w,
        plusSurface->h
    };
    SDL_RenderCopy(renderer, plusTexture, NULL, &plusTextRect);
    SDL_FreeSurface(plusSurface);
    SDL_DestroyTexture(plusTexture);

    x += (BUTTON_WIDTH / 2) + BUTTON_SPACING;

    SDL_Rect minusButton = { x, y, BUTTON_WIDTH / 2, BUTTON_HEIGHT };
    SDL_SetRenderDrawColor(renderer, 10, 182, 139, 255);
    SDL_RenderFillRect(renderer, &minusButton);

    SDL_Surface *minusSurface = TTF_RenderText_Solid(font, "-", textColor);
    SDL_Texture *minusTexture = SDL_CreateTextureFromSurface(renderer, minusSurface);
    SDL_Rect minusTextRect = {
        minusButton.x + ((BUTTON_WIDTH / 2) - minusSurface->w) / 2,
        minusButton.y + (BUTTON_HEIGHT - minusSurface->h) / 2,
        minusSurface->w,
        minusSurface->h
    };
    SDL_RenderCopy(renderer, minusTexture, NULL, &minusTextRect);
    SDL_FreeSurface(minusSurface);
    SDL_DestroyTexture(minusTexture);
}

void renderGame(SDL_Renderer *renderer) {
    for (int y = 0; y < GRID_HEIGHT / CELL_SIZE; y++) {
        for (int x = 0; x < GRID_WIDTH / CELL_SIZE; x++) {
            SDL_Rect rect = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            Particle *particle = &grid[y][x];

            switch (particle->type)
            {
            case PARTICLE_SAND:
                SDL_SetRenderDrawColor(renderer, particle->color.r, particle->color.g, particle->color.b, 255);
                break;

            case PARTICLE_WATER:
                SDL_SetRenderDrawColor(renderer, particle->color.r, particle->color.g, particle->color.b, 255);
                break;

            case PARTICLE_WET_SAND:
                SDL_SetRenderDrawColor(renderer, 174, 143, 96, 255);
                break;

            case PARTICLE_SMOKE:
                SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
                break;

            case PARTICLE_FIRE:
                SDL_SetRenderDrawColor(renderer, 255, 126, 0, 255);
                break;

            case PARTICLE_STONE:
                SDL_SetRenderDrawColor(renderer, particle->color.r, particle->color.g, particle->color.b, 255);
                break;

            case PARTICLE_CLEAR:
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                break;

            case PARTICLE_OIL:
                SDL_SetRenderDrawColor(renderer, 54, 17, 2, 255);
                break;

            case PARTICLE_LAVA:
                SDL_SetRenderDrawColor(renderer, particle->color.r, particle->color.g, particle->color.b, 255);
                break;

            case PARTICLE_OBSIDIAN:
                SDL_SetRenderDrawColor(renderer, 6, 3, 19, 255);
                break;

            case PARTICLE_STEAM:
                SDL_SetRenderDrawColor(renderer, particle->color.r, particle->color.g, particle->color.b, 255);
                break;

            default:
                continue;
            }

            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void renderGameStats(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Surface *fpsSurface = TTF_RenderText_Solid(font, fpsText, (SDL_Color){ 232, 141, 103, 255 });
    if (!fpsSurface) 
        return;

    SDL_Texture *fpsTexture = SDL_CreateTextureFromSurface(renderer, fpsSurface);
    if (!fpsTexture) {
        SDL_FreeSurface(fpsSurface);
        return;
    }

    SDL_Rect fpsTextRect = { 10, 10, fpsSurface->w, fpsSurface->h };
    SDL_RenderCopy(renderer, fpsTexture, NULL, &fpsTextRect);

    char brushSizeText[20];
    sprintf(brushSizeText, "Brush: %d", brushSize);

    SDL_Surface *brushSurface = TTF_RenderText_Solid(font, brushSizeText, (SDL_Color){ 232, 141, 103, 255 });
    if (!brushSurface) 
        return;

    SDL_Texture *brushTexture = SDL_CreateTextureFromSurface(renderer, brushSurface);
    if (!brushTexture) {
        SDL_FreeSurface(brushSurface);
        return;
    }

    SDL_Rect brushTextRect = { 10, 20 + fpsSurface->h, brushSurface->w, brushSurface->h };
    SDL_RenderCopy(renderer, brushTexture, NULL, &brushTextRect);

    SDL_FreeSurface(fpsSurface);
    SDL_DestroyTexture(fpsTexture);
    SDL_FreeSurface(brushSurface);
    SDL_DestroyTexture(brushTexture);
}

void render(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_SetRenderDrawColor(renderer, 243, 230, 230, 255);
    SDL_RenderClear(renderer);

    renderGameStats(renderer, font);
    renderButtons(renderer, font);
    renderGame(renderer);
    SDL_RenderPresent(renderer);
}

void update() {
    updateParticles();
}

void cleanUp(GameContext* pContext) {
    TTF_CloseFont(pContext->font);
    TTF_Quit();

    SDL_DestroyRenderer(pContext->renderer);
    SDL_DestroyWindow(pContext->window);
    SDL_Quit();
}