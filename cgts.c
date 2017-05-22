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

bool cgts_analyze_ts_packet(uint8_t * buf) {
    printf("%02x\n", buf[0]);
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
