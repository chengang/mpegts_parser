#include "structs.h"

/* context */
// todo..
struct cgts_context * cgts_alloc_with_memory(uint8_t * buf) {
    struct cgts_context * context = calloc(1, sizeof(struct cgts_context));
    context->ccounter = -1;
    context->tsp_counter = 0;
    return context;
}

struct cgts_context * cgts_alloc_with_file(const char * filename) {
    struct cgts_context * context = calloc(1, sizeof(struct cgts_context));
    context->input_type = CGTS_INPUT_TYPE_FILE;
    context->input_fp = fopen(filename, "r");
    return context;
}

void cgts_free(struct cgts_context * context) {
    if (context->input_type == CGTS_INPUT_TYPE_FILE) {
        fclose(context->input_fp);
    }
}

/* ts packet */
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
