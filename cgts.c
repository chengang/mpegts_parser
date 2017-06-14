#include "cgts.h"

bool cgts_sdt_parse(struct cgts_context * ct, const uint8_t * buf) {
    printf("SDT found\n");
    exit(0);
    return true;
}

bool cgts_pmt_parse(struct cgts_context * ct, struct cgts_pid_buffer * pid_buf) {
    //printf("PMT found\n");
    uint8_t * p = pid_buf->buf;
    uint16_t program_id = (p[0] << 8) | p[1];
    uint8_t version = (p[2] >> 1) & 0x1f;
    uint8_t sec_num = p[3];
    uint8_t last_sec_num = p[4];
    //printf("program_id:[%d], version: [%d], sec_num: [%d], last_sec_num:[%d]\n", program_id, version, sec_num, last_sec_num);

    int16_t pcr_pid = ((p[5] << 8) | p[6] ) & 0x1fff;
    int16_t program_info_length = ((p[7] << 8) | p[8] ) & 0x0fff;
    //printf("pcr_pid:[%d], program_info_length: [%d]\n", pcr_pid, program_info_length);
    //exit(0);

    int16_t ptr_moved = 7 + program_info_length;
    if (ptr_moved >= pid_buf->buf_pos) {
        return false;
    }

    //cgts_pid_buffer_print_hex(pid_buf);
    uint8_t * remain_buf = p + 9 + program_info_length; /* skip program info length */
    int32_t remain_buf_len = pid_buf->buf_pos - 9 /* bytes before pid map in pmt */ - program_info_length;
    while(remain_buf_len >= 5 /* min pid desc length is 40 bit = 5 byte */ ) {
        /********************************************/
        /* stream type  -- 8  bit                   */
        /* reserved     -- 3  bit                   */
        /* pid          -- 13 bit                   */
        /* reserved     -- 4  bit                   */
        /* info length  -- 12 bit                   */
        /* info         -- (info length) * 8  bit   */
        /********************************************/

        uint16_t read_bytes = 0;
        int16_t stream_type = remain_buf[0];
        if (stream_type < 0) {
            break;
        } else {
            read_bytes += 1;
        }

        int32_t pid = (remain_buf[1] << 8) | remain_buf[2];
        if (pid < 0) {
            break;
        } else {
            read_bytes += 2;
            pid &= 0x1fff;
        }

        int32_t es_info_length = (remain_buf[3] << 8) | remain_buf[4];
        if (es_info_length < 0) {
            break;
        } else {
            read_bytes += 2;
            es_info_length &= 0x0fff;
        }

        int32_t program_index = cgts_programs_index(ct, program_id);
        if (cgts_program_pid_exist(ct->programs[program_index], pid) == false) {
            cgts_program_pid_add(ct->programs[program_index], pid, stream_type);
        }

        remain_buf = remain_buf + read_bytes + es_info_length;
        remain_buf_len = remain_buf_len - read_bytes - es_info_length;
    }

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
        //printf("progid:[%d], pid:[%d]\n", program_id, pmt_pid);
        i += 4;

        // fill PMT`s pid into context
        if (cgts_programs_exists(ct, program_id) == false) {
            cgts_program_create(ct, program_id, pmt_pid);
        } else {
            cgts_program_delete(ct, program_id, pmt_pid);
            cgts_program_create(ct, program_id, pmt_pid);
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

        if (adaptation_flags & 0x10) {   // PCR_flag, means a PCR is in this tspacket
            if (adaptation_len < 7) { // PCR need 6btye + 1byte flags
                return false;
            }
            int64_t pcr_high = buf[6] * 256 * 256 * 256 + buf[7] * 256 * 256 + buf[8] * 256 + buf[9];
            pcr_high = (pcr_high << 1) | (buf[10] >> 7);
            int pcr_low = ((buf[10] & 1) << 8) | buf[11];
            tsp->pcr = pcr_high * 300 + pcr_low;
        }
        //fprintf(stdout, "pcr: %lld\n", tsp->pcr);

        ts_payload = ts_payload + adaptation_len /* afc length */ + 1 /* afc header */;
        ts_payload_len = ts_payload_len - adaptation_len - 1;
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
    if (! (ts_payload[0] == 0x00 && ts_payload[1] == 0x00 && ts_payload[2] == 0x01 ) ) {
        fprintf(stdout, "PES header error\n");
        return false;
    }
    pid_buf->stream_id = ts_payload[3];
    pid_buf->expect_len = (ts_payload[4] << 8) | ts_payload[5];
    //printf("expect length in header:[%d]\n", pid_buf->expect_len);
    return cgts_pid_buffer_append(pid_buf
            , ts_payload + 6 /* 6 Btyes = 3 (packet_start_code_prefix) + 1 (stream_id) + 2 (PES_packet_length) */
            , ts_payload_len - 6 );
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

    // check this TS packet complete the PXX packet, if not skip following processes
    if (cgts_pid_buffer_complete(ct->pid_buf[pid_buffer_index]) == false) {
        return true;
    }

    switch (pid_type) {
        case CGTS_PID_TYPE_PAT:
            cgts_pat_parse(ct, ct->pid_buf[pid_buffer_index]);
            return true;
            break;
        case CGTS_PID_TYPE_PMT:
            cgts_pmt_parse(ct, ct->pid_buf[pid_buffer_index]);
            return true;
            break;
        case CGTS_PID_TYPE_PES:
            printf("hihi, i am pes!\n");
            break;
    }


    // go on HERE!
    // go on HERE!
    // start parse PES!
    // if pid in pmts and is pes

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
