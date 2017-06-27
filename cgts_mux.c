#include "cgts_mux.h"

bool cgts_write_pxx_packet(struct cgts_mux_context * ct, struct cgts_pid_buffer * pid_buf) {
    uint32_t wrote_bytes = 0;
    bool ret = false;
    if (pid_buf->type == PXX_BUF_TYPE_PSI) {
        ret = cgts_write_psi_packet_header(ct, pid_buf, & wrote_bytes);
    } else if (pid_buf->type == PXX_BUF_TYPE_PES) {
        ret = cgts_write_pes_packet_header(ct, pid_buf, & wrote_bytes);
    }

    if (ret == false) {
        return false;
    } else if (wrote_bytes > pid_buf->expect_len){
        return false;
    } else if (wrote_bytes == pid_buf->expect_len){
        return true;
    }

    ret = cgts_write_pxx_packet_payload(ct, pid_buf, wrote_bytes);
    return ret;
}


bool cgts_write_psi_packet_header(struct cgts_mux_context * ct, struct cgts_pid_buffer * pid_buf, uint32_t * wrote_bytes) {
    if (pid_buf->type != PXX_BUF_TYPE_PSI) {
        return false;
    }

    uint32_t header_buf_len = 0;
    if (pid_buf->expect_len <= CGTS_TS_PACKET_SIZE - CGTS_TS_PACKET_HEADER_SIZE - CGTS_PSI_PACKET_HEADER_SIZE) {    // means: a single ts packet can delive this psi packet
        header_buf_len = pid_buf->expect_len + CGTS_PSI_PACKET_HEADER_SIZE;
    } else {
        header_buf_len = CGTS_TS_PACKET_SIZE - CGTS_TS_PACKET_HEADER_SIZE;
    }

    uint8_t * header_buf = calloc(1, header_buf_len);

    // first byte: pointer_field
    header_buf[0] = 0x00;

    // second byte: table_id
    header_buf[1] = pid_buf->table_id;

    // 3th and 4th bytes: expect_len
    header_buf[3] = pid_buf->expect_len % 256;
    header_buf[2] = (pid_buf->expect_len - (pid_buf->expect_len % 256) ) / 256;

    // CGTS_PSI_PACKET_HEADER_SIZE == 4
    memcpy(header_buf + CGTS_PSI_PACKET_HEADER_SIZE, pid_buf->buf, header_buf_len - CGTS_PSI_PACKET_HEADER_SIZE);

    cgts_write_ts_packet(ct, true, pid_buf->pid, header_buf, header_buf_len, wrote_bytes);
    (*wrote_bytes) = (*wrote_bytes) - CGTS_PSI_PACKET_HEADER_SIZE;

    free(header_buf);

    return true;
}

bool cgts_write_pes_packet_header(struct cgts_mux_context * ct, struct cgts_pid_buffer * pid_buf, uint32_t * wrote_bytes) {
    if (pid_buf->type != PXX_BUF_TYPE_PES) {
        return false;
    }

    uint32_t header_buf_len = 0;
    if (pid_buf->expect_len < CGTS_TS_PACKET_SIZE - CGTS_TS_PACKET_HEADER_SIZE - CGTS_PES_PACKET_HEADER_SIZE) {
        header_buf_len = pid_buf->expect_len + CGTS_PES_PACKET_HEADER_SIZE;
    } else {
        header_buf_len = CGTS_TS_PACKET_SIZE - CGTS_TS_PACKET_HEADER_SIZE;
    }

    uint8_t * header_buf = calloc(1, header_buf_len);

    // first three bytes: pes start bytes
    header_buf[0] = 0x00;
    header_buf[1] = 0x00;
    header_buf[2] = 0x01;

    // 4th byte: stream_id
    header_buf[3] = pid_buf->stream_id;

    // 5th and 6th bytes: expect_len
    header_buf[5] = pid_buf->expect_len % 256;
    header_buf[4] = (pid_buf->expect_len - (pid_buf->expect_len % 256) ) / 256;

    //printf("[%d],[%02x][%02x]\n", pid_buf->expect_len, header_buf[4], header_buf[5]);

    // CGTS_PES_PACKET_HEADER_SIZE == 6
    memcpy(header_buf + CGTS_PES_PACKET_HEADER_SIZE, pid_buf->buf, header_buf_len - CGTS_PES_PACKET_HEADER_SIZE);

    cgts_write_ts_packet(ct, true, pid_buf->pid, header_buf, header_buf_len, wrote_bytes);
    (*wrote_bytes) = (*wrote_bytes) - CGTS_PES_PACKET_HEADER_SIZE;

    free(header_buf);
    return true;
}

bool cgts_write_pxx_packet_payload(struct cgts_mux_context *ct, struct cgts_pid_buffer * pid_buf, uint32_t pid_buf_offset) {
    uint32_t wrote_bytes = 0;
    while(true) {
        cgts_write_ts_packet(ct, false, pid_buf->pid, pid_buf->buf + pid_buf_offset, pid_buf->expect_len - pid_buf_offset, &wrote_bytes);
        pid_buf_offset = pid_buf_offset + wrote_bytes;
        if (pid_buf_offset == pid_buf->expect_len) {
            break;
        } else if (pid_buf_offset > pid_buf->expect_len) {
            return false;
        }
    }
    return true;
}

bool cgts_write_ts_packet(struct cgts_mux_context * ct, bool is_pes_start, uint16_t pid, uint8_t * payload, uint16_t payload_len, uint32_t * wrote_bytes) {
    uint8_t * tsp_buf = calloc(1, CGTS_TS_PACKET_SIZE);

    // first byte: sync byte
    tsp_buf[0] = CGTS_SYNC_BYTE;

    // 2th and 3th bytes: start indicator AND pid
    tsp_buf[2] = pid % 256;
    tsp_buf[1] = (pid - pid % 256) / 256;
    tsp_buf[1] = tsp_buf[1] & 0x1f;
    if (is_pes_start == true) {
        tsp_buf[1] = tsp_buf[1] | 0x40;
    } else {
        tsp_buf[1] = tsp_buf[1] | 0x00;
    }

    // 4th byte: continuity counter AND adaotation field control AND scrambling control
    tsp_buf[3] = ct->ccounter;
    tsp_buf[3] = tsp_buf[3] & 0x0f;
    tsp_buf[3] = tsp_buf[3] | 0x00; // scrambling control: no scrambling
    tsp_buf[3] = tsp_buf[3] | 0x10; // adaotation field control : payload only

    uint16_t tsp_buf_len = 0;
    if (payload_len <= CGTS_TS_PACKET_SIZE - CGTS_TS_PACKET_HEADER_SIZE) {
        (*wrote_bytes) = payload_len;
        tsp_buf_len = CGTS_TS_PACKET_HEADER_SIZE + payload_len;
    } else {
        (*wrote_bytes) = CGTS_TS_PACKET_SIZE - CGTS_TS_PACKET_HEADER_SIZE;
        tsp_buf_len = CGTS_TS_PACKET_SIZE;
    }

    for (int i=CGTS_TS_PACKET_HEADER_SIZE;i<CGTS_TS_PACKET_SIZE;i++) {
        if (i >= tsp_buf_len ) {
            tsp_buf[i] = 0xff;
        } else {
            tsp_buf[i] = payload[i - CGTS_TS_PACKET_HEADER_SIZE];
        }
    }

    cgts_write_bytes(ct, tsp_buf, CGTS_TS_PACKET_SIZE);
    ct->tsp_counter = ct->tsp_counter + 1;
    if (ct->ccounter == 15) {
        ct->ccounter = 0;
    } else {
        ct->ccounter = ct->ccounter + 1;
    }

    free(tsp_buf);

    return true;
}

bool cgts_write_bytes(struct cgts_mux_context * ct, uint8_t * buf, uint32_t buf_len) {
    if (ct->output_type == CGTS_CONTEXT_OUTPUT_TYPE_FILE) {
        fwrite(buf, buf_len, 1, ct->output_fp);
    } else if (ct->output_type == CGTS_CONTEXT_OUTPUT_TYPE_MEMORY) {
        // todo...
        return false;
    } else {
        return false;
    }
    return true;
}
