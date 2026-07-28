#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[]) {
    // initialize only the video subsystem of SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_ERROR: %s\n", SDL_GetError());
        return 1;
    }

    // printf("SDL successfully initialized\n");
    // create the desktop window wrapper
    SDL_Window *window = SDL_CreateWindow(
        "taya custom vPlayer",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        800, 600, // default window width and height
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        printf("Error: Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // create the h/w accelerated renderer inside window
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    clock_t start_time = clock();
    clock_t end_time = start_time + (60 * CLOCKS_PER_SEC);

    while (clock() < end_time) {
        // delay for 1 minute
    }

    printf("SDL Window and Renderer successfully created\n");


    // clean up and shut down SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
