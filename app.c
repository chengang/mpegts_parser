#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cgts.h"

int main(int argc, char *argv[]) {
    fprintf(stderr, "ts filename:[%s]\n", argv[1]);
    struct cgts_context * context = cgts_alloc_with_file(argv[1]);
    cgts_parse(context);
    cgts_free(context);
    return 0;
}
