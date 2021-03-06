#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cgts_demux.h"
#include "cgts_mux.h"

typedef struct cgts_pid_buffer cgts_pxx_packet;

int main(int argc, char *argv[]) {

    // Init
    char * input_filename = argv[1];
    fprintf(stderr, "input filename: \t%s\n", input_filename);
    struct cgts_demux_context * demux_ct = cgts_demux_context_alloc_with_file(input_filename);

    // Work
    /*********************************************************/

    // --- parse mode ---
    //cgts_parse(demux_ct);

    // --- mpegts optimize mode ---
    //if (cgts_find_pat_and_pmt(demux_ct) == false) {
    //    printf("can not find pat or pmt\n");
    //    return 1;
    //}

    /*********************************************************/


    // --- remux mode ---
    char * output_filename = argv[2];
    fprintf(stderr, "output filename: \t%s\n", output_filename);
    struct cgts_mux_context * mux_ct = cgts_mux_context_alloc_with_file(output_filename);
    cgts_pxx_packet * packet = NULL;

    cgts_mux_context_debug(mux_ct);

    while(cgts_read_pxx_packet(demux_ct, &packet) == true) {
        cgts_pid_buffer_debug(packet);
        cgts_write_pxx_packet(mux_ct, packet);
    }

    cgts_mux_context_free(mux_ct);

    // Finalize
    cgts_demux_context_free(demux_ct);

    return 0;
}
