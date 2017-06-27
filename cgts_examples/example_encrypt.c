#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cgts_demux.h"
#include "cgts_mux.h"

typedef struct cgts_pid_buffer cgts_pxx_packet; // pxx means PSI or PES

int main(int argc, char *argv[]) {

    char * input_filename = argv[1];
    char * output_filename = argv[2];
    fprintf(stderr, "input filename: \t%s\n", input_filename);
    fprintf(stderr, "output filename: \t%s\n", output_filename);

    struct cgts_demux_context * demux_ct = cgts_demux_context_alloc_with_file(input_filename);
    struct cgts_mux_context * mux_ct = cgts_mux_context_alloc_with_file(output_filename);

    cgts_pxx_packet * packet = NULL;
    while(cgts_read_pxx_packet(demux_ct, &packet) == true) {
        cgts_pid_buffer_debug(packet);
        print_hex(packet->buf + packet->payload_offset, packet->expect_len - packet->payload_offset);

        // check ts packet type
        if (packet->type != PXX_BUF_TYPE_PSI && packet->type != PXX_BUF_TYPE_PES) {
            fprintf(stderr, "error: unknown ts packet type.\n"
                    "exit.\n");
            return 0;
        }

        // check audio and video codec
        if (packet->type == PXX_BUF_TYPE_PES &&
                (packet->stream_id != CGTS_STREAM_ID_PRIVATE_STREAM_1                   // AAC in some case
                 && packet->stream_id != CGTS_STREAM_ID_AUDIO_MPEG1_MPEG2_MPEG4_AAC     // AAC
                 && packet->stream_id != CGTS_STREAM_ID_VIDEO_MPEG1_MPEG2_MPEG4_AVC     // AVC
                 ) ) {
            fprintf(stderr, "error: encrypt AAC audio and AVC video only, illegal ES format found.\n"
                    "exit.\n");
            return 0;
        }

        // parse and encrypt AVC video
        if (packet->type == PXX_BUF_TYPE_PES 
                && packet->stream_id == CGTS_STREAM_ID_VIDEO_MPEG1_MPEG2_MPEG4_AVC
                ) {
        }

        // parse and encrypt AAC audio
        if (packet->type == PXX_BUF_TYPE_PES &&
                ( packet->stream_id == CGTS_STREAM_ID_AUDIO_MPEG1_MPEG2_MPEG4_AAC
                  || packet->stream_id == CGTS_STREAM_ID_PRIVATE_STREAM_1
                ) ) {
        }

        cgts_write_pxx_packet(mux_ct, packet);
    }

    cgts_mux_context_free(mux_ct);
    cgts_demux_context_free(demux_ct);

    return 0;
}

