#ifndef VPLAYER_H
#define VPLAYER_H

#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <stdio.h>

// helper struct to hold active SDL graphics context
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} SDL_PlayerContext;

// initializes SDL and allocates window/renderer/texture contexts
int init_sdl_context(SDL_PlayerContext *ctx, int width, int height);

// safely destroys all allocated SDL context resources
void destroy_sdl_context(SDL_PlayerContext *ctx);

#endif