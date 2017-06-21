#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cgts.h"

int main(int argc, char *argv[]) {
    fprintf(stderr, "ts filename:[%s]\n", argv[1]);
    struct cgts_context * context = cgts_alloc_with_file(argv[1]);
    if (cgts_find_pat_and_pmt(context) == false) {
        printf("can not find pat or pmt\n");
        return 1;
    }
    cgts_context_debug(context);
    //cgts_parse(context);
    cgts_free(context);
    return 0;
}
