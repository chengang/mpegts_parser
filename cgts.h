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

struct cgts_context * cgts_alloc_with_memory(uint8_t * buf);
struct cgts_context * cgts_alloc_with_file(const char * filename);
void cgts_free(struct cgts_context * context);

bool cgts_get188(struct cgts_context * context, uint8_t * buf);
void cgts_parse(struct cgts_context * context);
