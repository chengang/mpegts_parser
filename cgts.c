#include "cgts.h"
// todo..
struct cgts_context * cgts_alloc(uint8_t * buf) {
    struct cgts_context * context = calloc(1, sizeof(struct cgts_context));
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

struct cgts_ts_packet * cgts_ts_packet_alloc() {
    return (struct cgts_ts_packet *) calloc(1, sizeof(struct cgts_ts_packet));
}

bool cgts_ts_packet_parse(struct cgts_ts_packet * tsp, uint8_t * buf) {
    tsp->sync_byte = buf[0];
    tsp->unit_start_indicator = (buf[1] & 0x40) >> 6;
    tsp->pid = (buf[1] & 0x1f) * 256 + buf[2];
    tsp->scrambling_control = (buf[3] >> 4) & 0xc;
    tsp->adaption_field_control = (buf[3] >> 4) & 0x3;
    tsp->continuity_counter = (buf[3] & 0xf);
    return true;
}

void cgts_ts_packet_free(struct cgts_ts_packet * tsp) {
    free(tsp);
}

bool cgts_get188_from_file(FILE * fp, uint8_t * buf) {
    int read_bytes = fread(buf, 1, CGTS_PACKET_SIZE, fp);
    if (read_bytes == CGTS_PACKET_SIZE) {
        return true;
    }
    return false;
}

bool cgts_get188(struct cgts_context * ct, uint8_t * buf) {
    if (ct->input_type == CGTS_INPUT_TYPE_FILE) {
        return cgts_get188_from_file(ct->input_fp, buf);
    } else {
        return false;
    }
}

void cgts_ts_packet_debug(struct cgts_ts_packet * tsp) {
    fprintf(stdout, "sync_byte:%x, start_flag:%d, pid:%d, cc:%d\n",
            tsp->sync_byte, tsp->unit_start_indicator, 
            tsp->pid, tsp->continuity_counter
            );
}

bool cgts_analyze_ts_packet(uint8_t * buf) {
    //printf("%02x\n", buf[187]);

    struct cgts_ts_packet * tsp = cgts_ts_packet_alloc();
    cgts_ts_packet_parse(tsp, buf);
    cgts_ts_packet_debug(tsp);
    cgts_ts_packet_free(tsp);
    return true;
}

void cgts_parse(struct cgts_context * context) {
    uint8_t ts_packet_buf[CGTS_PACKET_SIZE];
    while(true) {
        if (cgts_get188(context, ts_packet_buf) == false) {
            break;
        }
        cgts_analyze_ts_packet(ts_packet_buf);
    }
}
