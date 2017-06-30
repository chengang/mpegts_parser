#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cgts_demux.h"
#include "cgts_mux.h"
#include "cgts_nal_adts_parse.h"
#include "aes128.h"

uint8_t key[16] = {
	 0x00, 0x00, 0x00, 0x00
	,0x00, 0x00, 0x00, 0x00
	,0x00, 0x00, 0x00, 0x00
	,0x00, 0x00, 0x00, 0x00
};

uint8_t iv[16] = {
	 0x00, 0x00, 0x00, 0x00
	,0x00, 0x00, 0x00, 0x00
	,0x00, 0x00, 0x00, 0x00
	,0x00, 0x00, 0x00, 0x00
};

bool encrypt_avc_es(struct cgts_pid_buffer * pid_buf) {
    uint32_t nalu_start_pos = 0;
    uint32_t nalu_end_pos = 0;

    uint8_t * encrypted_es = calloc(1, (2 * pid_buf->expect_len + 1024) );
    uint32_t encrypted_es_pos = 0;

    // memcpy PES header
    memcpy(encrypted_es + encrypted_es_pos, pid_buf->buf, pid_buf->payload_offset);
    encrypted_es_pos = pid_buf->payload_offset;

    uint32_t payload_start_pos = pid_buf->payload_offset;
    while(find_nal_unit(pid_buf->buf, pid_buf->expect_len, payload_start_pos, &nalu_start_pos, &nalu_end_pos) == true) {
        uint8_t nalu_type = pid_buf->buf[(nalu_start_pos + 4)] & 0x1f;
        if (nalu_type == CGTS_NAL_TYPE_IDR_SLICE) {
            // IDR data
            // first 5 bytes is NAL header, DO NOT encrypt them.
            memcpy(encrypted_es + encrypted_es_pos
                    , pid_buf->buf + nalu_start_pos
                    , CGTS_NAL_HEADER_SIZE);
            encrypted_es_pos = encrypted_es_pos + CGTS_NAL_HEADER_SIZE;

            // encrypt NAL payload bytes
            uint32_t payload_len = nalu_end_pos - nalu_start_pos + 1 - CGTS_NAL_HEADER_SIZE;
            uint8_t * buf4enc = calloc(1, payload_len + 16 /* AES128 block size */);
            uint32_t buf4enc_len = AES128_CBC_encrypt_buffer(
                    buf4enc
                    , pid_buf->buf + nalu_start_pos + CGTS_NAL_HEADER_SIZE
                    , payload_len
                    , key, iv);
            memcpy(encrypted_es + encrypted_es_pos
                    , buf4enc
                    , buf4enc_len);
            encrypted_es_pos = encrypted_es_pos + buf4enc_len;
			free(buf4enc);
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

bool encrypt_aac_es(struct cgts_pid_buffer * pid_buf) {
    uint32_t adtsu_start_pos = 0;
    uint32_t adtsu_end_pos = 0;

    uint8_t * encrypted_es = calloc(1, (2 * pid_buf->expect_len + 1024) );
    uint32_t encrypted_es_pos = 0;

    // memcpy PES header
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

        // first 7 bytes is ADTS header, DO NOT encrypt them.
        memcpy(encrypted_es + encrypted_es_pos
                , pid_buf->buf + adtsu_start_pos
                , CGTS_ADTS_HEADER_SIZE);
        encrypted_es_pos = encrypted_es_pos + CGTS_ADTS_HEADER_SIZE;

        // encrypt ADTS payload bytes
		uint32_t payload_len = adtsu_end_pos - adtsu_start_pos + 1 - CGTS_ADTS_HEADER_SIZE;
		uint8_t * buf4enc = calloc(1, payload_len + 16 /* AES128 block size */);
		uint32_t buf4enc_len = AES128_CBC_encrypt_buffer(
				buf4enc
				, pid_buf->buf + adtsu_start_pos + CGTS_ADTS_HEADER_SIZE
				, payload_len
				, key, iv);
		memcpy(encrypted_es + encrypted_es_pos
				, buf4enc
				, buf4enc_len);
        encrypted_es_pos = encrypted_es_pos + buf4enc_len;
        free(buf4enc);

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

