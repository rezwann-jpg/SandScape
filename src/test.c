#include "game.h"

#define FPS 60
#define FPS_CAP

int main() {
    SDL_Renderer *renderer;
    SDL_Window *window;
    TTF_Font *font;

    intializeGame(&window, &renderer, &font);

    SDL_Event event;
    bool running = true;

    const double freq_ms = SDL_GetPerformanceFrequency();
    Uint64 last_time = SDL_GetPerformanceCounter();

    unsigned int frame_counter = 0;
    double frame_timer = last_time;

    bool isDragging = false;
    int prevMouseX = -1, prevMouseY = -1;

    while (running) {
        Uint64 current_time = SDL_GetPerformanceCounter();
        double delta = (current_time - last_time) / freq_ms * 1000.0;

        if (current_time > frame_timer + freq_ms) {
            sprintf(fpsText, "FPS: %d", frame_counter);
            frame_counter = 0;
            frame_timer = current_time;
        }

        handleEvents(&event, &running, &isDragging, &prevMouseX, &prevMouseY);

#ifdef FPS_CAP
        const double frame_delta = 1000.0 / FPS;
        if (delta > frame_delta)
#endif
        {
            update();
            render(renderer, font);

            last_time = current_time;
            ++frame_counter;
        }
    }

    cleanUp(window, renderer, font);

    return 0;
}