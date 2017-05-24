#include "cgts.h"
// todo..
struct cgts_context * cgts_alloc(uint8_t * buf) {
    struct cgts_context * context = calloc(1, sizeof(struct cgts_context));
    context->ccounter = -1;
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
    struct cgts_ts_packet * tsp = (struct cgts_ts_packet *) calloc(1, sizeof(struct cgts_ts_packet));
    return tsp;
}

bool cgts_ts_packet_parse(struct cgts_context * ct, struct cgts_ts_packet * tsp, uint8_t * buf) {
    tsp->sync_byte = buf[0];
    tsp->unit_start_indicator = (buf[1] & 0x40) >> 6;
    tsp->pid = (buf[1] & 0x1f) * 256 + buf[2];
    tsp->scrambling_control = (buf[3] >> 4) & 0xc;
    tsp->adaption_field_control = (buf[3] >> 4) & 0x3;
    tsp->continuity_counter = (buf[3] & 0xf);

    tsp->has_adaptation = (tsp->adaption_field_control & 2) >> 1;
    tsp->has_payload = tsp->adaption_field_control & 1;

    if (tsp->has_adaptation) {
        uint8_t adaptation_len = buf[4];
        uint8_t adaptation_flags = buf[5];

        if (!(adaptation_flags & 0x10)) {   // PCR_flag
            return false;
        }
        if (adaptation_len < 7) { // PCR need 6btye + 1byte flags
            return false;
        }
        int64_t pcr_high = buf[6] * 256 * 256 * 256 + buf[7] * 256 * 256 + buf[8] * 256 + buf[9];
        pcr_high = (pcr_high << 1) | (buf[10] >> 7);
        int pcr_low = ((buf[10] & 1) << 8) | buf[11];
        ct->pcr = pcr_high * 300 + pcr_low;
        fprintf(stdout, "pcr: %lld\n", ct->pcr);
    }

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
    fprintf(stdout, "sync_byte:%x, start_flag:%d, pid:%d, cc:%d, payload:%d, adap:%d\n",
            tsp->sync_byte, tsp->unit_start_indicator, 
            tsp->pid, tsp->continuity_counter,
            tsp->has_payload, tsp->has_adaptation
            );
}

bool cgts_analyze_ts_packet(struct cgts_context * ct, uint8_t * buf) {
    //printf("%02x\n", buf[187]);

    struct cgts_ts_packet * tsp = cgts_ts_packet_alloc();
    cgts_ts_packet_parse(ct, tsp, buf);
    cgts_ts_packet_debug(tsp);
    cgts_ts_packet_free(tsp);
    return true;
}

void cgts_parse(struct cgts_context * ct) {
    uint8_t ts_packet_buf[CGTS_PACKET_SIZE];
    while(true) {
        if (cgts_get188(ct, ts_packet_buf) == false) {
            break;
        }
        cgts_analyze_ts_packet(ct, ts_packet_buf);
    }
}
