#include "structs.h"


/**********************************/
/*************** pxx **************/
/**********************************/

#define PXX_BUF_LEN_DEFAULT 1024
struct cgts_pxx_buffer * cgts_pxx_buffer_alloc(uint16_t pid) {
    struct cgts_pxx_buffer * pxx_buf = calloc(1, sizeof(struct cgts_pxx_buffer));
    pxx_buf->pid = pid;
    pxx_buf->buf = calloc(1, PXX_BUF_LEN_DEFAULT);
    pxx_buf->buf_pos = 0;
    pxx_buf->buf_cap = PXX_BUF_LEN_DEFAULT;
    return pxx_buf;
}

void cgts_pxx_buffer_free(struct cgts_pxx_buffer * pxx_buf) {
    free(pxx_buf->buf);
    free(pxx_buf);
}

bool cgts_pxx_buffer_append(struct cgts_pxx_buffer * pxx_buf, uint8_t * ts_payload, uint32_t ts_payload_len) {
    while (pxx_buf->buf_pos + ts_payload_len >= pxx_buf->buf_cap) {
        pxx_buf->buf = realloc(pxx_buf->buf, pxx_buf->buf_cap * 2);
        if (pxx_buf->buf == NULL) {
            return false;
        }
        pxx_buf->buf_cap = pxx_buf->buf_cap * 2;
    }
    memcpy(pxx_buf->buf + pxx_buf->buf_pos, ts_payload, ts_payload_len);
    pxx_buf->buf_pos +=  ts_payload_len;

    return true;
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
    context->pxx_buf_num = 1;
    context->pxx_buf[(context->pxx_buf_num - 1)] = cgts_pxx_buffer_alloc(0);
    return context;
}

void cgts_free(struct cgts_context * context) {
    if (context->input_type == CGTS_INPUT_TYPE_FILE) {
        fclose(context->input_fp);
    }
    for(int i=0; i < context->pxx_buf_num; i++) {
        cgts_pxx_buffer_free(context->pxx_buf[i]);
    }
}

bool cgts_pid_exists(struct cgts_context * ct, uint16_t pid) {
    for (int i=0;i<ct->pxx_buf_num;i++) {
        if (pid == ct->pxx_buf[i]->pid) {
            return true;
        }
    }
    return false;
}

bool cgts_pid_create(struct cgts_context * ct, uint16_t pid) {
   if (ct->pxx_buf_num >= MAX_PIDS_IN_SIGNLE_MPEGTS) {
       return false;
   }

   ct->pxx_buf_num = ct->pxx_buf_num + 1;
   ct->pxx_buf[(ct->pxx_buf_num - 1)] = cgts_pxx_buffer_alloc(pid);

   return true;
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
