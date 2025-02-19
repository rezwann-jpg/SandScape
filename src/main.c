#include "game.h"

const int FPS = 60;

GameContext* g_pContext = NULL;

int main() {
    g_pContext = (GameContext *)malloc(sizeof(GameContext));

    intializeGame(g_pContext);

    const double freq_ms = SDL_GetPerformanceFrequency();
    Uint64 last_time = SDL_GetPerformanceCounter();

    unsigned int frame_counter = 0;
    double frame_timer = last_time;

    while (g_pContext->running) {
        Uint64 current_time = SDL_GetPerformanceCounter();
        double delta = (current_time - last_time) / freq_ms * 1000.0;

        if (current_time > frame_timer + freq_ms) {
            sprintf(fpsText, "FPS: %d", frame_counter);
            frame_counter = 0;
            frame_timer = current_time;
        }

        handleEvents(g_pContext);

        const double frame_delta = 1000.0 / FPS;
        if (delta > frame_delta) {
            update();
            render(g_pContext->renderer, g_pContext->font);

            last_time = current_time;
            ++frame_counter;
        }
    }

    cleanUp(g_pContext);

    free(g_pContext);

    return 0;
}