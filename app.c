#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cgts_demux.h"

typedef struct cgts_pid_buffer cgts_pxx_packet;

int main(int argc, char *argv[]) {

    // Init
    fprintf(stderr, "ts filename:[%s]\n", argv[1]);
    struct cgts_demux_context * context = cgts_demux_context_alloc_with_file(argv[1]);

    // Work
    /*********************************************************/

    // --- parse mode ---
    //cgts_parse(context);

    // --- mpegts optimize mode ---
    //if (cgts_find_pat_and_pmt(context) == false) {
    //    printf("can not find pat or pmt\n");
    //    return 1;
    //}

    /*********************************************************/


    // --- remux mode ---
    cgts_pxx_packet * packet = NULL;
    while(cgts_read_pxx_packet(context, &packet) == true) {
        cgts_pid_buffer_debug(packet);
    }

    // Finalize
    cgts_demux_context_free(context);
    return 0;
}
