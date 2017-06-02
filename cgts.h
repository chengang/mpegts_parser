#ifndef __CGTS_H__
#define __CGTS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "util.h"

#define CGTS_STREAM_TYPE_VIDEO_MPEG1     0x01
#define CGTS_STREAM_TYPE_VIDEO_MPEG2     0x02
#define CGTS_STREAM_TYPE_AUDIO_MPEG1     0x03
#define CGTS_STREAM_TYPE_AUDIO_MPEG2     0x04
#define CGTS_STREAM_TYPE_PRIVATE_SECTION 0x05
#define CGTS_STREAM_TYPE_PRIVATE_DATA    0x06
#define CGTS_STREAM_TYPE_AUDIO_AAC       0x0f
#define CGTS_STREAM_TYPE_AUDIO_AAC_LATM  0x11
#define CGTS_STREAM_TYPE_VIDEO_MPEG4     0x10
#define CGTS_STREAM_TYPE_METADATA        0x15
#define CGTS_STREAM_TYPE_VIDEO_H264      0x1b
#define CGTS_STREAM_TYPE_VIDEO_HEVC      0x24
#define CGTS_STREAM_TYPE_VIDEO_CAVS      0x42
#define CGTS_STREAM_TYPE_VIDEO_VC1       0xea
#define CGTS_STREAM_TYPE_VIDEO_DIRAC     0xd1

#define CGTS_STREAM_TYPE_AUDIO_AC3       0x81
#define CGTS_STREAM_TYPE_AUDIO_DTS       0x82
#define CGTS_STREAM_TYPE_AUDIO_TRUEHD    0x83
#define CGTS_STREAM_TYPE_AUDIO_EAC3      0x87

bool cgts_pxx_packet_append(struct cgts_context * ct, uint16_t pid, bool is_start, const uint8_t * ts_payload, uint32_t ts_payload_len);

bool cgts_sdt_parse(struct cgts_context * ct, const uint8_t * buf);
bool cgts_pmt_parse(struct cgts_context * ct, struct cgts_pid_buffer * pid_buf);
bool cgts_pat_parse(struct cgts_context * ct, struct cgts_pid_buffer * pid_buf);
bool cgts_ts_packet_payload_parse(struct cgts_context * ct, const uint8_t * buf);
bool cgts_ts_packet_parse(struct cgts_context * ct, struct cgts_ts_packet * tsp, uint8_t * buf);
bool cgts_analyze_ts_packet(struct cgts_context * ct, uint8_t * buf);

bool cgts_get188_from_file(FILE * fp, uint8_t * buf);
bool cgts_get188(struct cgts_context * context, uint8_t * buf);
void cgts_parse(struct cgts_context * context);

#endif
