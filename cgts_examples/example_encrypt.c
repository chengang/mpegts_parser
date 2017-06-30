#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cgts_demux.h"
#include "cgts_mux.h"

typedef struct cgts_pid_buffer cgts_pxx_packet; // pxx means PSI or PES

bool find_nal_unit(uint8_t * buf, uint32_t buf_len, uint32_t buf_start_pos, uint32_t * nal_start_pos, uint32_t * nal_end_pos) {
    bool nalu_start_found = false;
    bool nalu_end_found = false;
    uint32_t nalu_start_pos = 0;
    uint32_t nalu_end_pos = 0;

    for (uint32_t i=buf_start_pos; i<buf_len-3; i++) {
        if ( buf[i] == 0x00 && buf[i+1] == 0x00
                && buf[i+2] == 0x00 && buf[i+3] == 0x01) 
        {
            if ( nalu_start_found == false) {
                nalu_start_pos = i;
                nalu_start_found = true;
            } else if ( nalu_end_found == false ) {
                nalu_end_pos = i - 1;
                nalu_end_found = true;
            }
            i = i + 4;
        }

        if ( i == buf_len 
                - 3 /* length of start code - 1 (4-1) */  
                - 1 /* last index equal length - 1 */ )
        {
            if ( nalu_start_found == true ) {
                nalu_end_pos = i + 3;
                nalu_end_found = true;
            }
        }

        if ( nalu_start_found == true && nalu_end_found == true ) {
            (* nal_start_pos) = nalu_start_pos;
            (* nal_end_pos) = nalu_end_pos;
            return true;
        }
    }
    return false;
}

bool find_adts_unit(uint8_t * buf, uint32_t buf_len, uint32_t buf_start_pos, uint32_t * adts_start_pos, uint32_t * adts_end_pos) {
    bool adtsu_start_found = false;
    bool adtsu_end_found = false;
    uint32_t adtsu_start_pos = 0;
    uint32_t adtsu_end_pos = 0;

    for (uint32_t i=buf_start_pos; i<buf_len-1; i++) {
        if ( buf[i] == 0xff 
                && buf[i+1] == 0xf1)
        {
            if ( adtsu_start_found == false) {
                adtsu_start_pos = i;
                adtsu_start_found = true;
            } else if ( adtsu_end_found == false ) {
                adtsu_end_pos = i - 1;
                adtsu_end_found = true;
            }
            i = i + 2;
        }

        if ( i == buf_len 
                - 1 /* length of start code - 1 (2-1)*/
                - 1 /* last index equal length - 1 */ )
        {
            if ( adtsu_start_found == true ) {
                adtsu_end_pos = i + 1;
                adtsu_end_found = true;
            }
        }

        if ( adtsu_start_found == true && adtsu_end_found == true ) {
            (* adts_start_pos) = adtsu_start_pos;
            (* adts_end_pos) = adtsu_end_pos;
            return true;
        }
    }
    return false;
}

#define CGTS_NAL_HEADER_SIZE   5
bool encrypt_avc_es(struct cgts_pid_buffer * pid_buf) {
    uint32_t nalu_start_pos = 0;
    uint32_t nalu_end_pos = 0;

    uint8_t * encrypted_es = calloc(1, (2 * pid_buf->expect_len + 1024) );
    uint32_t encrypted_es_pos = 0;

    memcpy(encrypted_es + encrypted_es_pos, pid_buf->buf, pid_buf->payload_offset);
    encrypted_es_pos = pid_buf->payload_offset;

    uint32_t payload_start_pos = pid_buf->payload_offset;
    while(find_nal_unit(pid_buf->buf, pid_buf->expect_len, payload_start_pos, &nalu_start_pos, &nalu_end_pos) == true) {
        uint8_t nalu_type = pid_buf->buf[(nalu_start_pos + 4)] & 0x1f;
        if (nalu_type == 0x05) {
            // IDR data
            // first 5 bytes is NAL header, DO NOT encrypt that.
            memcpy(encrypted_es + encrypted_es_pos
                    , pid_buf->buf + nalu_start_pos
                    , CGTS_NAL_HEADER_SIZE);
            encrypted_es_pos = encrypted_es_pos + CGTS_NAL_HEADER_SIZE;

            // encrypt nal payload bytes
            memcpy(encrypted_es + encrypted_es_pos
                    , pid_buf->buf + nalu_start_pos + CGTS_NAL_HEADER_SIZE
                    , nalu_end_pos - nalu_start_pos + 1 - CGTS_NAL_HEADER_SIZE);
            encrypted_es_pos = encrypted_es_pos + ( nalu_end_pos - nalu_start_pos + 1 - CGTS_NAL_HEADER_SIZE );

            memcpy(encrypted_es + encrypted_es_pos
                    , pid_buf->buf + nalu_start_pos
                    , nalu_end_pos - nalu_start_pos + 1);
            encrypted_es_pos = encrypted_es_pos + nalu_end_pos - nalu_start_pos + 1;
        } else {
            // not IDR data
            memcpy(encrypted_es + encrypted_es_pos
                    , pid_buf->buf + nalu_start_pos
                    , nalu_end_pos - nalu_start_pos + 1);
            encrypted_es_pos = encrypted_es_pos + nalu_end_pos - nalu_start_pos + 1;
        }
        payload_start_pos = nalu_end_pos;
        //printf("nalu start at:[%d], end at:[%d]\n", nalu_start_pos, nalu_end_pos);
    }

    //print_hex(encrypted_es, encrypted_es_pos);
    cgts_pid_buffer_overwrite(pid_buf, encrypted_es, encrypted_es_pos);
    pid_buf->expect_len = encrypted_es_pos;

    return true;
}

#define CGTS_ADTS_HEADER_SIZE   7
bool encrypt_aac_es(struct cgts_pid_buffer * pid_buf) {
    uint32_t adtsu_start_pos = 0;
    uint32_t adtsu_end_pos = 0;

    uint8_t * encrypted_es = calloc(1, (2 * pid_buf->expect_len + 1024) );
    uint32_t encrypted_es_pos = 0;

    memcpy(encrypted_es + encrypted_es_pos, pid_buf->buf, pid_buf->payload_offset);
    encrypted_es_pos = pid_buf->payload_offset;

    uint32_t payload_start_pos = pid_buf->payload_offset;
    while(find_adts_unit(pid_buf->buf, pid_buf->expect_len, payload_start_pos, &adtsu_start_pos, &adtsu_end_pos) == true) {
        /* 
         * checking calculation frame length code 

        uint16_t frame_length = 
            ((pid_buf->buf[adtsu_start_pos + 3] & 0x03) << 14)
            | (pid_buf->buf[adtsu_start_pos + 4] << 3)
            | ((pid_buf->buf[adtsu_start_pos + 5] & 0xe0) >> 5);

        printf("start: [%d], end: [%d], length: [%d]=[%d]\n"
                , adtsu_start_pos, adtsu_end_pos
                , frame_length
                , adtsu_end_pos - adtsu_start_pos + 1);

        */

        // first 7 bytes is ADTS header, DO NOT encrypt that.
        memcpy(encrypted_es + encrypted_es_pos
                , pid_buf->buf + adtsu_start_pos
                , CGTS_ADTS_HEADER_SIZE);
        encrypted_es_pos = encrypted_es_pos + CGTS_ADTS_HEADER_SIZE;

        // encrypt adts payload bytes
        memcpy(encrypted_es + encrypted_es_pos
                , pid_buf->buf + adtsu_start_pos + CGTS_ADTS_HEADER_SIZE
                , adtsu_end_pos - adtsu_start_pos + 1 - CGTS_ADTS_HEADER_SIZE);
        encrypted_es_pos = encrypted_es_pos + ( adtsu_end_pos - adtsu_start_pos + 1 - CGTS_ADTS_HEADER_SIZE );

        payload_start_pos = adtsu_end_pos;
        //printf("adtsu start at:[%d], end at:[%d]\n", adtsu_start_pos, adtsu_end_pos);
    }

    //print_hex(encrypted_es, encrypted_es_pos);
    cgts_pid_buffer_overwrite(pid_buf, encrypted_es, encrypted_es_pos);
    pid_buf->expect_len = encrypted_es_pos;
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
        //print_hex(packet->buf + packet->payload_offset, packet->expect_len - packet->payload_offset);

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

        // parse and encrypt AAC audio
        if (packet->type == PXX_BUF_TYPE_PES &&
                ( packet->stream_id == CGTS_STREAM_ID_AUDIO_MPEG1_MPEG2_MPEG4_AAC
                  || packet->stream_id == CGTS_STREAM_ID_PRIVATE_STREAM_1
                ) ) 
        {
            encrypt_aac_es(packet);
        }

        // parse and encrypt AVC video
        if (packet->type == PXX_BUF_TYPE_PES 
                && packet->stream_id == CGTS_STREAM_ID_VIDEO_MPEG1_MPEG2_MPEG4_AVC
                ) 
        {
            encrypt_avc_es(packet);
        }

        cgts_write_pxx_packet(mux_ct, packet);
    }

    cgts_mux_context_free(mux_ct);
    cgts_demux_context_free(demux_ct);

    return 0;
}
