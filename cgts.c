#include "cgts.h"

void print_hex(uint8_t * buf, int len) {
  int i;
  for(i=0;i<len;i++) {
    fprintf(stderr, " %02x", buf[i]);
    if (i % 8 == 7) {
      fprintf(stderr, " ");
    }
    if (i % 16 == 15) {
      fprintf(stderr, "\n");
    }
  }
  fprintf(stderr, "\n");
}

// todo..
struct cgts_context * cgts_alloc(uint8_t * buf) {
    struct cgts_context * context = calloc(1, sizeof(struct cgts_context));
    context->ccounter = -1;
    context->tsp_counter = 0;
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
    tsp->pcr = 0;
    return tsp;
}

bool cgts_pat_parse(struct cgts_context * ct, const uint8_t * buf) {
    printf("[%x][%x]", buf[0], buf[2]);
    uint8_t pointer_field = buf[0];
    const uint8_t * p = buf + pointer_field + 1;
    uint8_t table_id = p[0];
    uint16_t section_length = (p[1] & 0x0f << 8) | p[2];
    printf("table_id:[%d], sec_len:[%d]\n", table_id, section_length);
    exit(0);
    return true;
}

bool cgts_ts_packet_parse(struct cgts_context * ct, struct cgts_ts_packet * tsp, uint8_t * buf) {
    ct->tsp_counter++;

    tsp->sync_byte = buf[0];
    tsp->unit_start_indicator = (buf[1] & 0x40) >> 6;
    tsp->pid = (buf[1] & 0x1f) * 256 + buf[2];
    tsp->scrambling_control = (buf[3] >> 4) & 0xc;
    tsp->adaption_field_control = (buf[3] >> 4) & 0x3;
    tsp->continuity_counter = (buf[3] & 0xf);

    ct->ccounter = tsp->continuity_counter;

    tsp->has_adaptation = (tsp->adaption_field_control & 2) >> 1;
    tsp->has_payload = tsp->adaption_field_control & 1;

    const uint8_t *p = buf + 4; /*ts header*/

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
        tsp->pcr = pcr_high * 300 + pcr_low;
        //fprintf(stdout, "pcr: %lld\n", tsp->pcr);

        p = p + p[0] /* afc length */ + 1 /* afc header */;
    }

    if (!(tsp->has_payload)) {
        return false;
    }
    if (p >= buf + CGTS_PACKET_SIZE) {
        return false;
    }

    /* TS Packet Header Finished */
    /* TS Packet Payload Start */

    if (tsp->pid == 0x00) {
        /* PAT */
        cgts_pat_parse(ct, p);
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

void cgts_ts_packet_debug(struct cgts_context * ct, struct cgts_ts_packet * tsp) {
    fprintf(stdout, "%d: sync_byte:%x, start_flag:%d, pid:%d, cc:%d, payload:%d, adap:%d, ",
            ct->tsp_counter,
            tsp->sync_byte, tsp->unit_start_indicator, 
            tsp->pid, tsp->continuity_counter,
            tsp->has_payload, tsp->has_adaptation
            );

    if (tsp->pid == 0) {
        fprintf(stdout, " PAT ");
    }
    if (tsp->pcr != 0) {
        fprintf(stdout, " PCR: %lld ", tsp->pcr);
    }
    fprintf(stdout, "\n");
}

bool cgts_analyze_ts_packet(struct cgts_context * ct, uint8_t * buf) {
    //printf("%02x\n", buf[187]);
	print_hex(buf, 188);

    struct cgts_ts_packet * tsp = cgts_ts_packet_alloc();
    cgts_ts_packet_parse(ct, tsp, buf);
    cgts_ts_packet_debug(ct, tsp);
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
