#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cgts.h"

int main(int argc, char *argv[]) {
    fprintf(stderr, "ts filename:[%s]\n", argv[1]);
    struct cgts_context * context = cgts_alloc_with_file(argv[1]);
    //cgts_parse(context);

    //if (cgts_find_pat_and_pmt(context) == false) {
    //    printf("can not find pat or pmt\n");
    //    return 1;
    //}
    //cgts_context_debug(context);

    struct cgts_pxx_packet * packet = cgts_pxx_packet_alloc();
    while(cgts_read_pxx_packet(context, packet) == true) {
        printf("hi\n");
    }

    cgts_pxx_packet_free(packet);
    cgts_free(context);
    return 0;
}
