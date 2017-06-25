#include "cgts_mux.h"

bool cgts_write_pxx_packet(struct cgts_mux_context * ct, struct cgts_pid_buffer * pid_buf) {

    int64_t bytes_wait_for_write_in = pid_buf->buf_pos;

    uint32_t wrote_bytes = 0;
    bool ret = false;
    if (pid_buf->type == PXX_BUF_TYPE_PSI) {
        ret = cgts_write_psi_packet_header(ct, pid_buf, & wrote_bytes);
    } else if (pid_buf->type == PXX_BUF_TYPE_PES) {
        ret = cgts_write_pes_packet_header(ct, pid_buf, & wrote_bytes);
    }

    if (ret == false) {
        return false;
    }

    bytes_wait_for_write_in = bytes_wait_for_write_in - wrote_bytes;
    while(true) {
        ret = cgts_write_pxx_packet_payload(ct, pid_buf, & wrote_bytes);
        if (ret == false) {
            return false;
        }
        printf("asasasa[%lld]\n", bytes_wait_for_write_in);
        bytes_wait_for_write_in = bytes_wait_for_write_in - wrote_bytes;
        if (bytes_wait_for_write_in <= 0) {
            break;
        }
    }
    return true;
}


bool cgts_write_psi_packet_header(struct cgts_mux_context * ct, struct cgts_pid_buffer * pid_buf, uint32_t * wrote_bytes) {
    *wrote_bytes = 10;
    return true;
}

bool cgts_write_pes_packet_header(struct cgts_mux_context * ct, struct cgts_pid_buffer * pid_buf, uint32_t * wrote_bytes) {
    *wrote_bytes = 10;
    return true;
}

bool cgts_write_pxx_packet_payload(struct cgts_mux_context *ct, struct cgts_pid_buffer * pid_buf, uint32_t * wrote_bytes) {
    *wrote_bytes = 10;
    return true;
}

bool cgts_write_ts_packet(struct cgts_mux_context * ct, bool is_pes_start, uint16_t pid, uint8_t * payload, uint16_t payload_len) {
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
