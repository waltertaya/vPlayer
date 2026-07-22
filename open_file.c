#include <libavformat/avformat.h>
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

    printf("Successfully opened %s!\n", argv[1]);


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

    printf("Video stream index: %d\n", video_stream_index);

    // clean up by closing the input context
    avformat_close_input(&format_context);
    return 0;
}
