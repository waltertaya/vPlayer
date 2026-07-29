#include "vplayer.h"

int init_sdl_context(SDL_PlayerContext *ctx, int width, int height) {
    // initialize the SDL Video Subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // create desktop window
    ctx->window = SDL_CreateWindow(
        "taya custom vPlayer",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width / 2, height / 2, // default window dimensions scaled comfortably
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!ctx->window) {
        printf("Error: Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // create hardware accelerated renderer
    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ctx->renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return -1;
    }

    // allocate streaming YUV texture in VRAM
    ctx->texture = SDL_CreateTexture(
        ctx->renderer,
        SDL_PIXELFORMAT_YV12,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );
    if (!ctx->texture) {
        printf("Texture creation failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
        return -1;
    }

    return 0;
}

void destroy_sdl_context(SDL_PlayerContext *ctx) {
    if (ctx->texture) {
        SDL_DestroyTexture(ctx->texture);
        ctx->texture = NULL;
    }
    if (ctx->renderer) {
        SDL_DestroyRenderer(ctx->renderer);
        ctx->renderer = NULL;
    }
    if (ctx->window) {
        SDL_DestroyWindow(ctx->window);
        ctx->window = NULL;
    }
    SDL_Quit();
}
