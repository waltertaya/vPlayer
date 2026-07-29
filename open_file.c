#include "vplayer.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <stdio.h>

static int handle_sdl_events(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return 0;
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // create out main container context pointer
    AVFormatContext *format_context = NULL;

    // open the media file and read its header
    if (avformat_open_input(&format_context, argv[1], NULL, NULL) < 0) {
        printf("Error: Could not open file %s\n", argv[1]);
        return -1;
    }

    // printf("Successfully opened %s!\n", argv[1]);

    // retrieve detailed stream information
    if (avformat_find_stream_info(format_context, NULL) < 0) {
        printf("Error: Could not retrieve stream info\n");
        avformat_close_input(&format_context);
        return -1;
    }

    // print format details to stdout for inspection
    // av_dump_format(format_context, 0, argv[1], 0);

    int video_stream_index = -1;

    // loop through all the streams in the container
    for (int i = 0; i < format_context->nb_streams; i++) {
        // check the codec type of the current stream
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        printf("Error: Could not find a video stream\n");
        avformat_close_input(&format_context);
        return -1;
    }

    // printf("Video stream index: %d\n", video_stream_index);
    // find the registered decoder for out specific video stream
    const AVCodec *codec = avcodec_find_decoder(format_context->streams[video_stream_index]->codecpar->codec_id);
    if (!codec) {
        printf("Error: Unsupported codec\n");
        avformat_close_input(&format_context);
        return -1;
    }

    // allocate an empty context our decoder
    AVCodecContext *codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        printf("Error: Could not allocate codec context\n");
        avformat_close_input(&format_context);
        return -1;
    }

    // copy stream's configurations into the decoder context
    if (avcodec_parameters_to_context(codec_context, format_context->streams[video_stream_index]->codecpar) < 0) {
        printf("Error: Could not copy codec parameters\n");
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    // open the decoder to make it ready for active decoding
    if (avcodec_open2(codec_context, codec, NULL) < 0) {
        printf("Error: Could not open codec\n");
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    // allocate an empty packet wrapper on the stack
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    if (!packet || !frame) {
        printf("Error: Failed to allocate packet or frame\n");
        if (packet) av_packet_free(&packet);
        if (frame) av_frame_free(&frame);
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    // initialize the graphics context prior to playback loop
    SDL_PlayerContext sdl_ctx;
    if (init_sdl_context(&sdl_ctx, codec_context->width, codec_context->height) < 0) {
        printf("Error: Failed to initialize SDL graphics pipeline\n");
        av_packet_free(&packet);
        av_frame_free(&frame);
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    // read packets in a continuous loop from the container
    int is_running = 1;
    while (is_running && av_read_frame(format_context, packet) >= 0) {
        is_running = handle_sdl_events();
        if (!is_running) {
            av_packet_unref(packet);
            break;
        }

        if (packet->stream_index == video_stream_index) {
            int response = avcodec_send_packet(codec_context, packet);
            if (response < 0) {
                printf("Error while sending a packet to the decoder\n");
                break;
            }

            while (is_running && response >= 0) {
                is_running = handle_sdl_events();
                if (!is_running) {
                    break;
                }

                response = avcodec_receive_frame(codec_context, frame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    break;
                } else if (response < 0) {
                    printf("Error receiving a frame from the decoder\n");
                    break;
                }

                // upload uncompressed frame data directly to GPU texture
                SDL_UpdateYUVTexture(
                    sdl_ctx.texture,
                    NULL,
                    frame->data[0], frame->linesize[0],  // Y
                    frame->data[1], frame->linesize[1],  // U
                    frame->data[2], frame->linesize[2]   // V
                );

                // paint frame to the screen
                SDL_RenderClear(sdl_ctx.renderer);
                SDL_RenderCopy(sdl_ctx.renderer, sdl_ctx.texture, NULL, NULL);
                SDL_RenderPresent(sdl_ctx.renderer);

                is_running = handle_sdl_events();
                if (!is_running) {
                    break;
                }

                // standard 40ms frame delay
                SDL_Delay(40);

                // decoded a frame successfully
                // printf("Decoded Frame %ld (Width: %d, Height: %d)\n", codec_context->frame_num, frame->width, frame->height);
            }
        }

        // wipe the packet's internal buffers to prepare for the next read
        av_packet_unref(packet);
    }

    // safe cleanup
    destroy_sdl_context(&sdl_ctx);
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codec_context);
    avformat_close_input(&format_context);

    // printf("Playback complete and memory cleaned up cleanly!\n");
    return 0;
}
