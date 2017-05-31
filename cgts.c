#include "cgts.h"

bool cgts_sdt_parse(struct cgts_context * ct, const uint8_t * buf) {
    printf("SDT found\n");
    exit(0);
    return true;
}

bool cgts_cat_parse(struct cgts_context * ct, const uint8_t * buf) {
    printf("CAT found\n");
    exit(0);
    return true;
}

bool cgts_pat_parse(struct cgts_context * ct, const uint8_t * buf) {
    uint8_t pointer_field = buf[0];
    const uint8_t * p = buf + pointer_field + 1;
    uint8_t table_id = p[0];
    uint16_t section_length = ( (p[1] & 0x0f) << 8) | p[2];
    uint16_t stream_id = (p[3] << 8) | p[4];
    uint8_t version = (p[5] >> 1) & 0x1f;
    uint8_t sec_num = p[6];
    uint8_t last_sec_num = p[7];
    //printf("table_id:[%d], sec_len:[%d]\n", table_id, section_length);

    uint16_t i = 0;
    while(true) {
        if (5 /* header(8) - bytes before section length(3) */ 
                + i /* prev id pair */ 
                + 4 /* this id pair*/ 
                + 4 /* crc32 */ 
                >= section_length + 3) {
            break;
        }
        uint16_t program_id = (p[8 /* header */ +i] << 8) | p[9+i]; 
        uint16_t pid = ((p[8+i+2] << 8) | p[9+i+2]) & 0x1fff; 
        //printf("progid:[%d], pid:[%d]\n", program_id, pid);
        i += 4;
    }

    return true;
}

bool cgts_ts_packet_payload_parse(struct cgts_context * ct, const uint8_t * buf) {
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

    if (tsp->unit_start_indicator) {
        cgts_pxx_packet_append(ct, tsp->pid, true, p);
    } else {
        cgts_pxx_packet_append(ct, tsp->pid, false, p);
    }

    return true;
}

bool cgts_pxx_packet_append(struct cgts_context * ct, uint16_t pid, bool is_start, const uint8_t * ts_payload) {
    if (cgts_pid_exists(ct, pid) == false) {
        if (cgts_pid_create(ct, pid) == false) {
            return false;
        }
    }
    // go on HERE!
    // go on HERE!
    // go on HERE!
    // go on HERE!
    // go on HERE!
    // go on HERE!
    // go on HERE!
    //cgts_append_buffer(ct, pid, ts_payload);
    return true;


#if 0
    if (tsp->pid == CGTS_PID_PAT) {
        /* PAT */
    } else if (tsp->pid == CGTS_PID_CAT) {
        /* CAT */
        cgts_cat_parse(ct, p);
    } else if (tsp->pid == CGTS_PID_SDT) {
        /* Transport Stream Desc Table */
        cgts_sdt_parse(ct, p);
    } else {
        if (tsp->unit_start_indicator == 0) {
            //append_payload_to_pid(buf);
        }
        /* NIT or PMT or PES */
        cgts_ts_packet_payload_parse(ct, p);
    }
#endif
}

bool cgts_analyze_ts_packet(struct cgts_context * ct, uint8_t * buf) {
    //printf("%02x\n", buf[187]);
	//print_hex(buf, 188);

    struct cgts_ts_packet * tsp = cgts_ts_packet_alloc();
    cgts_ts_packet_parse(ct, tsp, buf);
    cgts_ts_packet_debug(ct, tsp);
    cgts_ts_packet_free(tsp);
    return true;
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

void cgts_parse(struct cgts_context * ct) {
    uint8_t ts_packet_buf[CGTS_PACKET_SIZE];
    while(true) {
        if (cgts_get188(ct, ts_packet_buf) == false) {
            break;
        }
        cgts_analyze_ts_packet(ct, ts_packet_buf);
    }
}
