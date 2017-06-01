#include "structs.h"

/**********************************/
/************ program *************/
/**********************************/

struct cgts_program * cgts_program_alloc(uint16_t program_id, uint16_t pmt_pid) {
    struct cgts_program * program = calloc(1, sizeof(struct cgts_program));
    program->program_id = program_id;
    program->pmt_pid = pmt_pid;
    program->pids_num = 0;
    return program;
}

void cgts_program_free(struct cgts_program * program) {
    free(program);
}

bool cgts_program_pid_add(struct cgts_program * program, uint16_t pid) {
    if (program->pids_num >= MAX_PIDS_PER_PROGRAM) {
        return false;
    }
    program->pids[program->pids_num] = pid;
    program->pids_num = program->pids_num + 1;
    return true;
}

/**********************************/
/*********** pid buffer ***********/
/**********************************/

#define PXX_BUF_LEN_DEFAULT 1024
struct cgts_pid_buffer * cgts_pid_buffer_alloc(uint16_t pid) {
    struct cgts_pid_buffer * pid_buf = calloc(1, sizeof(struct cgts_pid_buffer));
    pid_buf->pid = pid;
    pid_buf->expect_len = 0;
    pid_buf->buf = calloc(1, PXX_BUF_LEN_DEFAULT);
    pid_buf->buf_pos = 0;
    pid_buf->buf_cap = PXX_BUF_LEN_DEFAULT;
    return pid_buf;
}

void cgts_pid_buffer_free(struct cgts_pid_buffer * pid_buf) {
    free(pid_buf->buf);
    free(pid_buf);
}

bool cgts_pid_buffer_append(struct cgts_pid_buffer * pid_buf, const uint8_t * ts_payload, uint32_t ts_payload_len) {
    while (pid_buf->buf_pos + ts_payload_len >= pid_buf->buf_cap) {
        pid_buf->buf = realloc(pid_buf->buf, pid_buf->buf_cap * 2);
        if (pid_buf->buf == NULL) {
            return false;
        }
        pid_buf->buf_cap = pid_buf->buf_cap * 2;
    }
    memcpy(pid_buf->buf + pid_buf->buf_pos, ts_payload, ts_payload_len);
    pid_buf->buf_pos +=  ts_payload_len;
    return true;
}

void cgts_pid_buffer_reset(struct cgts_pid_buffer * pid_buf) {
    memset(pid_buf->buf, 0, pid_buf->buf_cap);
    pid_buf->expect_len = 0;
    pid_buf->buf_pos = 0;
}

bool cgts_pid_buffer_complete(struct cgts_pid_buffer * pid_buf) {
    if (pid_buf->buf_pos == pid_buf->expect_len) {
        return true;
    } else {
        return false;
    }
}

/**********************************/
/************ context *************/
/**********************************/

// todo.. start
struct cgts_context * cgts_alloc_with_memory(uint8_t * buf) {
    struct cgts_context * context = calloc(1, sizeof(struct cgts_context));
    return context;
}
// todo.. end

struct cgts_context * cgts_alloc_with_file(const char * filename) {
    struct cgts_context * context = calloc(1, sizeof(struct cgts_context));
    context->input_type = CGTS_INPUT_TYPE_FILE;
    context->input_fp = fopen(filename, "r");
    context->programs_num = 0;
    context->pid_buf_num = 1;
    context->pid_buf[(context->pid_buf_num - 1)] = cgts_pid_buffer_alloc(0);
    return context;
}

void cgts_free(struct cgts_context * context) {
    if (context->input_type == CGTS_INPUT_TYPE_FILE) {
        fclose(context->input_fp);
    }
    for(int i=0; i < context->pid_buf_num; i++) {
        cgts_pid_buffer_free(context->pid_buf[i]);
    }
}

bool cgts_pid_exists(struct cgts_context * ct, uint16_t pid) {
    for (int i=0;i<ct->pid_buf_num;i++) {
        if (pid == ct->pid_buf[i]->pid) {
            return true;
        }
    }
    return false;
}

int32_t cgts_pid_buffer_index(struct cgts_context * ct, uint16_t pid) {
    for (int i=0;i<ct->pid_buf_num;i++) {
        if (pid == ct->pid_buf[i]->pid) {
            return i;
        }
    }
    return -1;
}

bool cgts_pid_create(struct cgts_context * ct, uint16_t pid) {
   if (ct->pid_buf_num >= MAX_PIDS_IN_SIGNLE_MPEGTS) {
       return false;
   }

   ct->pid_buf_num = ct->pid_buf_num + 1;
   ct->pid_buf[(ct->pid_buf_num - 1)] = cgts_pid_buffer_alloc(pid);

   return true;
}

int16_t cgts_pid_type(struct cgts_context * ct, uint16_t pid) {
    if (pid == CGTS_PID_PAT) {
        return CGTS_PID_TYPE_PAT;
    }

    if (pid == CGTS_PID_CAT) {
        return CGTS_PID_TYPE_PSI;
    }

    if (pid == CGTS_PID_SDT) {
        return CGTS_PID_TYPE_PSI;
    }

    for (int i=0;i<ct->programs_num;i++) {
        struct cgts_program * prog = ct->programs[i];
        if (pid == prog->pmt_pid) {
            return CGTS_PID_TYPE_PMT;
        }
        for (int j=0;j<prog->pids_num;j++) {
            if (pid == prog->pids[j]) {
                return CGTS_PID_TYPE_PES;
            }
        }
    }
    return CGTS_PID_TYPE_UNKNOWN;
}

/**********************************/
/************ ts packet ***********/
/**********************************/

struct cgts_ts_packet * cgts_ts_packet_alloc() {
    struct cgts_ts_packet * tsp = (struct cgts_ts_packet *) calloc(1, sizeof(struct cgts_ts_packet));
    tsp->pcr = 0;
    return tsp;
}

void cgts_ts_packet_free(struct cgts_ts_packet * tsp) {
    free(tsp);
}

void cgts_ts_packet_debug(struct cgts_context * ct, struct cgts_ts_packet * tsp) {
    fprintf(stdout, "%d: sync_byte:%x, start_flag:%d, pid:%d, cc:%d, payload:%d, adap:%d, ",
            ct->tsp_counter,
            tsp->sync_byte, tsp->unit_start_indicator, 
            tsp->pid, tsp->continuity_counter,
            tsp->has_payload, tsp->has_adaptation
            );

    if (tsp->pid == 0) {
        fprintf(stdout, " PAT ");
    }
    if (tsp->pcr != 0) {
        fprintf(stdout, " PCR: %lld ", tsp->pcr);
    }
    fprintf(stdout, "\n");
}
