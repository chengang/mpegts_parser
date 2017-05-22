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

bool cgts_get188(struct cgts_context * context, uint8_t * buf) {
    return true;
}

void cgts_parse(struct cgts_context * context) {
    uint8_t ts_packet_buf[CGTS_PACKET_SIZE];
    while(1) {
        cgts_get188(context, ts_packet_buf);
    }
}
