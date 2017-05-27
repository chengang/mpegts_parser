#ifndef __CGTS_STRUCTS_H__
#define __CGTS_STRUCTS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define CGTS_PACKET_SIZE 188
#define CGTS_INPUT_TYPE_FILE 1
#define CGTS_INPUT_TYPE_MEMORY 2

#define CGTS_PID_PAT 0x00
#define CGTS_PID_CAT 0x01
#define CGTS_PID_SDT 0x02

#define MAX_PIDS_PER_PROGRAM 64
struct cgts_program {
    uint16_t program_id;
    uint16_t pmt_pid;
    uint16_t pids[MAX_PIDS_PER_PROGRAM];
    uint8_t pids_num;
};

/* context */
struct cgts_context {
    uint8_t input_type; // 1-file, 2-memory
    FILE * input_fp;
    uint8_t * input_ptr;
    uint32_t tsp_counter;
    int8_t ccounter;

    struct cgts_program * programs;
    uint16_t programs_num;
};

struct cgts_context * cgts_alloc_with_memory(uint8_t * buf);
struct cgts_context * cgts_alloc_with_file(const char * filename);
void cgts_free(struct cgts_context * context);

/* ts packet */
struct cgts_ts_packet {
    uint8_t sync_byte;
    uint8_t unit_start_indicator;
    uint16_t pid;
    uint8_t scrambling_control;
    uint8_t adaption_field_control;
    uint8_t continuity_counter;

    uint8_t has_adaptation;
    uint8_t has_payload;

    uint64_t pcr;
};

struct cgts_ts_packet * cgts_ts_packet_alloc();
void cgts_ts_packet_free(struct cgts_ts_packet * tsp);
void cgts_ts_packet_debug(struct cgts_context * ct, struct cgts_ts_packet * tsp);

struct cgts_pat {
    uint8_t table_id;
    uint16_t stream_id;
    uint8_t version;
    uint8_t sec_num;
    uint8_t last_sec_num;
    struct cgts_program *prg;
};

#endif