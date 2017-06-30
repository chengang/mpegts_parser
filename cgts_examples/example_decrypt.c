#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cgts_demux.h"
#include "cgts_mux.h"
#include "cgts_nal_adts_parse.h"

bool decrypt_avc_es(struct cgts_pid_buffer * pid_buf) {
    uint32_t nalu_start_pos = 0;
    uint32_t nalu_end_pos = 0;

    uint8_t * decrypted_es = calloc(1, (2 * pid_buf->expect_len + 1024) );
    uint32_t decrypted_es_pos = 0;

    // memcpy PES header
    memcpy(decrypted_es + decrypted_es_pos, pid_buf->buf, pid_buf->payload_offset);
    decrypted_es_pos = pid_buf->payload_offset;

    uint32_t payload_start_pos = pid_buf->payload_offset;
    while(find_nal_unit(pid_buf->buf, pid_buf->expect_len, payload_start_pos, &nalu_start_pos, &nalu_end_pos) == true) {
        uint8_t nalu_type = pid_buf->buf[(nalu_start_pos + 4)] & 0x1f;
        if (nalu_type == CGTS_NAL_TYPE_IDR_SLICE) {
            // IDR data
            // first 5 bytes is NAL header, DO NOT decrypt them.
            memcpy(decrypted_es + decrypted_es_pos
                    , pid_buf->buf + nalu_start_pos
                    , CGTS_NAL_HEADER_SIZE);
            decrypted_es_pos = decrypted_es_pos + CGTS_NAL_HEADER_SIZE;

            // decrypt NAL payload bytes
            /*
             * Do decrypting here
             *
            memcpy(decrypted_es + decrypted_es_pos
                    , pid_buf->buf + nalu_start_pos + CGTS_NAL_HEADER_SIZE
                    , nalu_end_pos - nalu_start_pos + 1 - CGTS_NAL_HEADER_SIZE);
            decrypted_es_pos = decrypted_es_pos + ( nalu_end_pos - nalu_start_pos + 1 - CGTS_NAL_HEADER_SIZE );
             *
             *
             */
        } else {
            // not IDR data
            // these data stay clear, just memcpy them
            memcpy(decrypted_es + decrypted_es_pos
                    , pid_buf->buf + nalu_start_pos
                    , nalu_end_pos - nalu_start_pos + 1);
            decrypted_es_pos = decrypted_es_pos + nalu_end_pos - nalu_start_pos + 1;
        }
        payload_start_pos = nalu_end_pos;
    }

    cgts_pid_buffer_overwrite(pid_buf, decrypted_es, decrypted_es_pos);
    pid_buf->expect_len = decrypted_es_pos;

    return true;
}

bool decrypt_aac_es(struct cgts_pid_buffer * pid_buf) {
    uint32_t adtsu_start_pos = 0;
    uint32_t adtsu_end_pos = 0;

    uint8_t * decrypted_es = calloc(1, (2 * pid_buf->expect_len + 1024) );
    uint32_t decrypted_es_pos = 0;

    // memcpy PES header
    memcpy(decrypted_es + decrypted_es_pos, pid_buf->buf, pid_buf->payload_offset);
    decrypted_es_pos = pid_buf->payload_offset;

    uint32_t payload_start_pos = pid_buf->payload_offset;
    while(find_adts_unit(pid_buf->buf, pid_buf->expect_len, payload_start_pos, &adtsu_start_pos, &adtsu_end_pos) == true) {
        // first 7 bytes is ADTS header, DO NOT decrypt them.
        memcpy(decrypted_es + decrypted_es_pos
                , pid_buf->buf + adtsu_start_pos
                , CGTS_ADTS_HEADER_SIZE);
        decrypted_es_pos = decrypted_es_pos + CGTS_ADTS_HEADER_SIZE;

        // decrypt ADTS payload bytes
        /*
         * Do decrypting here
         *
        memcpy(decrypted_es + decrypted_es_pos
                , pid_buf->buf + adtsu_start_pos + CGTS_ADTS_HEADER_SIZE
                , adtsu_end_pos - adtsu_start_pos + 1 - CGTS_ADTS_HEADER_SIZE);
        decrypted_es_pos = decrypted_es_pos + ( adtsu_end_pos - adtsu_start_pos + 1 - CGTS_ADTS_HEADER_SIZE );
        *
        *
        */

        payload_start_pos = adtsu_end_pos;
    }

    cgts_pid_buffer_overwrite(pid_buf, decrypted_es, decrypted_es_pos);
    pid_buf->expect_len = decrypted_es_pos;
    return true;
}

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

        // check ts packet type
        if (packet->type != PXX_BUF_TYPE_PSI && packet->type != PXX_BUF_TYPE_PES) {
            fprintf(stderr, "error: unknown mpegts packet type.\n"
                    "exit.\n");
            return 0;
        }

        // check audio and video codec
        if (packet->type == PXX_BUF_TYPE_PES &&
                (packet->stream_id != CGTS_STREAM_ID_PRIVATE_STREAM_1                   // AAC in some case
                 && packet->stream_id != CGTS_STREAM_ID_AUDIO_MPEG1_MPEG2_MPEG4_AAC     // AAC
                 && packet->stream_id != CGTS_STREAM_ID_VIDEO_MPEG1_MPEG2_MPEG4_AVC     // AVC
                 ) ) {
            fprintf(stderr, "error: AAC audio and AVC video supported only, illegal ES format found.\n"
                    "exit.\n");
            return 0;
        }

        // parse and decrypt AAC audio
        if (packet->type == PXX_BUF_TYPE_PES &&
                ( packet->stream_id == CGTS_STREAM_ID_AUDIO_MPEG1_MPEG2_MPEG4_AAC
                  || packet->stream_id == CGTS_STREAM_ID_PRIVATE_STREAM_1
                ) ) 
        {
            decrypt_aac_es(packet);
        }

        // parse and decrypt AVC video
        if (packet->type == PXX_BUF_TYPE_PES 
                && packet->stream_id == CGTS_STREAM_ID_VIDEO_MPEG1_MPEG2_MPEG4_AVC
                ) 
        {
            decrypt_avc_es(packet);
        }

        cgts_write_pxx_packet(mux_ct, packet);
    }

    cgts_mux_context_free(mux_ct);
    cgts_demux_context_free(demux_ct);

    return 0;
}

