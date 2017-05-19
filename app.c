#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define CGTS_PACKET_SIZE 188
#define CGTS_INPUT_TYPE_FILE 1
#define CGTS_INPUT_TYPE_MEMORY 2

struct cgts_context {
    uint8_t input_type; // 1-file, 2-memory
    FILE * input_fp;
    uint8_t * input_ptr;
};

// todo..
struct cgts_context * cgts_init_with_memory(uint8_t * buf) {
    struct cgts_context * context = calloc(1, sizeof(struct cgts_context));
    return context;
}

struct cgts_context * cgts_init_with_file(const char * filename) {
    struct cgts_context * context = calloc(1, sizeof(struct cgts_context));
    context->input_type = CGTS_INPUT_TYPE_FILE;
    context->input_fp = fopen(filename, "r");
    return context;
}

void cgts_finish(struct cgts_context * context) {
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

int main(int argc, char *argv[]) {
    fprintf(stderr, "ts filename:[%s]\n", argv[1]);
    struct cgts_context * context = cgts_init(argv[1]);
    cgts_parse(context);
    return 0;
}
