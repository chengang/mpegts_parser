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

#define CGTS_PID_TYPE_PAT       0x10
#define CGTS_PID_TYPE_PMT       0x11
#define CGTS_PID_TYPE_PSI       0x12
#define CGTS_PID_TYPE_PES       0x13
#define CGTS_PID_TYPE_UNKNOWN   0x19

/* program */
#define MAX_PIDS_PER_PROGRAM 64
struct cgts_program {
    uint16_t program_id;
    uint16_t pmt_pid;
    uint16_t pids[MAX_PIDS_PER_PROGRAM];
    uint8_t pids_num;
};

struct cgts_program * cgts_program_alloc(uint16_t program_id, uint16_t pmt_pid);
void cgts_program_free(struct cgts_program * program);
bool cgts_program_pid_add(struct cgts_program * program, uint16_t pid);

/* pid buffer */
#define PXX_BUF_LEN_DEFAULT 1024
struct cgts_pid_buffer {
    uint16_t pid;
    uint8_t table_id;
    uint32_t expect_len;
    uint8_t * buf;
    uint32_t buf_pos;
    uint32_t buf_cap;
};

struct cgts_pid_buffer * cgts_pid_buffer_alloc(uint16_t pid);
void cgts_pid_buffer_free(struct cgts_pid_buffer * pid_buf);
bool cgts_pid_buffer_append(struct cgts_pid_buffer * pid_buf, const uint8_t * ts_payload, uint32_t ts_payload_len);
void cgts_pid_buffer_reset(struct cgts_pid_buffer * pid_buf);
bool cgts_pid_buffer_complete(struct cgts_pid_buffer * pid_buf);

/* context */
#define MAX_PIDS_IN_SIGNLE_MPEGTS       512
#define MAX_PROGRAMS_IN_SIGNLE_MPEGTS   512
#define CGTS_CONTEXT_INPUT_TYPE_FILE    1
#define CGTS_CONTEXT_INPUT_TYPE_MEMORY  2
struct cgts_context {
    uint8_t input_type; // 1-file, 2-memory
    FILE * input_fp;
    uint8_t * input_ptr;
    uint32_t tsp_counter;
    int8_t ccounter;

    struct cgts_program * programs[MAX_PROGRAMS_IN_SIGNLE_MPEGTS];
    uint16_t programs_num;

    struct cgts_pid_buffer * pid_buf[MAX_PIDS_IN_SIGNLE_MPEGTS];
    uint16_t pid_buf_num;
};

struct cgts_context * cgts_alloc_with_memory(uint8_t * buf);
struct cgts_context * cgts_alloc_with_file(const char * filename);
void cgts_free(struct cgts_context * context);
void cgts_context_debug(struct cgts_context * ct);

bool cgts_programs_exists(struct cgts_context * ct, uint16_t prog_id);
int32_t cgts_programs_index(struct cgts_context * ct, uint16_t prog_id);
bool cgts_program_create(struct cgts_context * ct, uint16_t prog_id, uint16_t pmt_pid);

bool cgts_pid_exists(struct cgts_context * ct, uint16_t pid);
int32_t cgts_pid_buffer_index(struct cgts_context * ct, uint16_t pid);
bool cgts_pid_create(struct cgts_context * ct, uint16_t pid);
int16_t cgts_pid_type(struct cgts_context * ct, uint16_t pid);

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
