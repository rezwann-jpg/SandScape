#include "game.h"

const int FPS = 60;

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
TTF_Font *g_font = NULL;

SDL_Event g_event;
bool g_running = true;

int main() {
    intializeGame(&g_window, &g_renderer, &g_font);

    const double freq_ms = SDL_GetPerformanceFrequency();
    Uint64 last_time = SDL_GetPerformanceCounter();

    unsigned int frame_counter = 0;
    double frame_timer = last_time;

    bool isDragging = false;
    int prevMouseX = -1, prevMouseY = -1;

    while (g_running) {
        Uint64 current_time = SDL_GetPerformanceCounter();
        double delta = (current_time - last_time) / freq_ms * 1000.0;

        if (current_time > frame_timer + freq_ms) {
            sprintf(fpsText, "FPS: %d", frame_counter);
            frame_counter = 0;
            frame_timer = current_time;
        }

        handleEvents(&g_event, &g_running, &isDragging, &prevMouseX, &prevMouseY);

        const double frame_delta = 1000.0 / FPS;
        if (delta > frame_delta) {
            update();
            render(g_renderer, g_font);

            last_time = current_time;
            ++frame_counter;
        }
    }

    cleanUp(g_window, g_renderer, g_font);

    return 0;
}