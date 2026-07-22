#include <SDL2/SDL.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    // initialize only the video subsystem of SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_ERROR: %s\n", SDL_GetError());
        return 1;
    }

    printf("SDL successfully initialized\n");

    // clean up and shut down SDL
    SDL_Quit();
    return 0;
}
