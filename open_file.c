#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <stdio.h>

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
    av_dump_format(format_context, 0, argv[1], 0);

    int video_stream_index = -1;

    // loop through all the streams in the container
    for (int i = 0; i < format_context->nb_streams; i++) {
        // check the codec type of the current stream
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    // printf("Video stream index: %d\n", video_stream_index);
    // Find the registered decoder for out specific video stream
    const AVCodec *codec = avcodec_find_decoder(format_context->streams[video_stream_index]->codecpar->codec_id);

    if (!codec) {
        printf("Error: Unsupported codec\n");
        return -1;
    }

    // allocate an empty context our decoder
    AVCodecContext *codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        printf("Error: Could not allocate codec context\n");
        return -1;
    }

    // copy our stream's configurations into the decoder context
    if (avcodec_parameters_to_context(codec_context, format_context->streams[video_stream_index]->codecpar) < 0) {
        printf("Error: Could not copy codec paramaters\n");
        return -1;
    }

    // open the decoder to make it ready for active decoding
    if (avcodec_open2(codec_context, codec, NULL) < 0) {
        printf("Error: Could not open codec\n");
        return -1;
    }

    // allocate an empty packet wrapper on the stack
    AVPacket *packet = av_packet_alloc();
    if (!packet) {
        printf("Error: Failed to allocate packet\n");
        return -1;
    }

    // allocate a reusable raw frame to hold the decoded pixels
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        printf("Could not allocate video frame\n");
        return -1;
    }

    // read packets in a continuous loop from the container
    while (av_read_frame(format_context, packet) >= 0) {
        // check if this packet belongs to our selected video stream
        if (packet->stream_index == video_stream_index) {
            // send compressed packet to the decoder
            int response = avcodec_send_packet(codec_context, packet);
            if (response < 0) {
                printf("Error while sending a packet to the decoder\n");
                return response;
            }
            // printf("Read video packet with size: %d bytes\n", packet->size);
            while (response >= 0) {
                // retrieve uncompressed frames from the decoder
                response = avcodec_receive_frame(codec_context, frame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    // EAGAIN => we need to feed the decoder more packets first
                    break;
                } else if (response < 0) {
                    printf("Error: receiving a frame from the decoder\n");
                    return response;
                }

                // decoded a frame successfully
                printf("Decoded Frame %ld (Width: %d, Height: %d)\n", codec_context->frame_num, frame->width, frame->height);
            }
        }

        // wipe the packet's internal buffers to prepare for the next read
        av_packet_unref(packet);
    }

    // clean up packet wrapper when playback ends
    av_packet_free(&packet);

    // clean up by closing the input context
    avformat_close_input(&format_context);
    return 0;
}
