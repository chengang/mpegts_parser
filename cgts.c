#include "cgts.h"

bool cgts_sdt_parse(struct cgts_context * ct, const uint8_t * buf) {
    printf("SDT found\n");
    exit(0);
    return true;
}

bool cgts_pmt_parse(struct cgts_context * ct, struct cgts_pid_buffer * pid_buf) {
    printf("PMT found\n");
    uint8_t * p = pid_buf->buf;
    uint16_t program_id = (p[0] << 8) | p[1];
    uint8_t version = (p[2] >> 1) & 0x1f;
    uint8_t sec_num = p[3];
    uint8_t last_sec_num = p[4];
    printf("program_id:[%d], version: [%d], sec_num: [%d], last_sec_num:[%d]\n", program_id, version, sec_num, last_sec_num);

    int16_t pcr_pid = ((p[5] << 8) | p[6] ) & 0x1fff;
    int16_t program_info_length = ((p[7] << 8) | p[8] ) & 0x0fff;
    printf("pcr_pid:[%d], program_info_length: [%d]\n", pcr_pid, program_info_length);
    //exit(0);

    int16_t ptr_moved = 7 + program_info_length;
    if (ptr_moved >= pid_buf->buf_pos) {
        return false;
    }
    p = p + 7 + program_info_length;

    // go on HERE!
    // go on HERE!
    // go on HERE!
    // go on HERE!
    // go on HERE!
    // go on HERE!
    // go on HERE!

    return true;
}

bool cgts_pat_parse(struct cgts_context * ct, struct cgts_pid_buffer * pid_buf) {
    if (pid_buf->pid != 0) {
        return false;
    }
    uint8_t * p = pid_buf->buf;
    uint16_t stream_id = (p[0] << 8) | p[1];
    uint8_t version = (p[2] >> 1) & 0x1f;
    uint8_t sec_num = p[3];
    uint8_t last_sec_num = p[4];
    //printf("table_id:[%d], sec_len:[%d]\n", table_id, section_length);

    uint16_t i = 0;
    while(true) {
        if (5 /* header(8) - bytes before section length(3) */ 
                + i /* prev id pair */ 
                + 4 /* this id pair*/ 
                + 4 /* crc32 */ 
                >= pid_buf->expect_len + 3) {
            break;
        }
        uint16_t program_id = 
            (p[5 /* header */ +i] << 8)             /* first byte */ 
            | p[6+i]                                /* second byte */ ; 
        uint16_t pmt_pid = 
            ((p[5+i+2] << 8)                        /* third byte */ 
             | p[6+i+2])                            /* fourth byte */ & 0x1fff; 
        printf("progid:[%d], pid:[%d]\n", program_id, pmt_pid);
        i += 4;

        // fill PMT`s pid into context
        if (cgts_programs_exists(ct, program_id) == false) {
            cgts_program_create(ct, program_id, pmt_pid);
        } else {
            // todo: delete program_id then cgts_program_create
        }
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

    int32_t ts_payload_len = CGTS_PACKET_SIZE;
    const uint8_t *ts_payload = buf + 4; /*ts header*/
    ts_payload_len = ts_payload_len - 4;

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

        ts_payload = ts_payload + adaptation_len /* afc length */ + 1 /* afc header */;
        ts_payload_len = ts_payload_len - adaptation_len;
    }

    if (!(tsp->has_payload)) {
        return false;
    }
    if (ts_payload >= buf + CGTS_PACKET_SIZE 
            || ts_payload_len <= 0) {
        return false;
    }

    /* TS Packet Header Finished */
    /* TS Packet Payload Start */

    if (tsp->unit_start_indicator) {
        cgts_pxx_packet_append(ct, tsp->pid, true, ts_payload, ts_payload_len);
    } else {
        cgts_pxx_packet_append(ct, tsp->pid, false, ts_payload, ts_payload_len);
    }

    return true;
}

bool cgts_pid_buffer_append_pes_header(struct cgts_pid_buffer * pid_buf, const uint8_t * ts_payload, uint32_t ts_payload_len) {
    pid_buf->expect_len = 999999;
    return cgts_pid_buffer_append(pid_buf, ts_payload, ts_payload_len);
}

bool cgts_pid_buffer_append_psi_header(struct cgts_pid_buffer * pid_buf, const uint8_t * ts_payload, uint32_t ts_payload_len) {
    uint8_t pointer_field = ts_payload[0];
    const uint8_t * p = ts_payload + pointer_field + 1;
    pid_buf->table_id = p[0];
    pid_buf->expect_len = ( (p[1] & 0x0f) << 8) | p[2];
    return cgts_pid_buffer_append(
            pid_buf
            , p + 3 /* skip table_id and section_length */
            , ts_payload_len - 1 /* pointer_field length */ - pointer_field - 3);
}

bool cgts_pxx_packet_append(struct cgts_context * ct, uint16_t pid, bool is_start, const uint8_t * ts_payload, uint32_t ts_payload_len) {
    if (cgts_pid_exists(ct, pid) == false) {
        if (cgts_pid_create(ct, pid) == false) {
            return false;
        }
    }


    //if (cgts_pid_buffer_append(ct->pid_buf[pid_buffer_index], ts_payload, ts_payload_len) == false) {
    //    return false;
    //}

    int16_t pid_type = cgts_pid_type(ct, pid);
    if (pid_type == CGTS_PID_TYPE_UNKNOWN) {
        return false;
    }

    int32_t pid_buffer_index = cgts_pid_buffer_index(ct, pid);
    if (pid_buffer_index == -1) {
        return false;
    }

    if (is_start == true) {
        cgts_pid_buffer_reset(ct->pid_buf[pid_buffer_index]);
        if (pid_type == CGTS_PID_TYPE_PAT
                || pid_type == CGTS_PID_TYPE_PMT
                || pid_type == CGTS_PID_TYPE_PSI ) {
            cgts_pid_buffer_append_psi_header(ct->pid_buf[pid_buffer_index], ts_payload, ts_payload_len);
        } else {
            cgts_pid_buffer_append_pes_header(ct->pid_buf[pid_buffer_index], ts_payload, ts_payload_len);
        }
    } else {
        cgts_pid_buffer_append(ct->pid_buf[pid_buffer_index], ts_payload, ts_payload_len);
    }

    // is packet complete ?
    if (cgts_pid_buffer_complete(ct->pid_buf[pid_buffer_index]) == false) {
        return true;
    }

    switch (pid_type) {
        case CGTS_PID_TYPE_PAT:
            cgts_pat_parse(ct, ct->pid_buf[pid_buffer_index]);
            break;
        case CGTS_PID_TYPE_PMT:
            cgts_pmt_parse(ct, ct->pid_buf[pid_buffer_index]);
            break;
    }

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
    cgts_context_debug(ct);
}
