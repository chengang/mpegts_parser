#include "cgts_nal_adts_parse.h"

bool find_nal_unit(uint8_t * buf, uint32_t buf_len, uint32_t buf_start_pos, uint32_t * nal_start_pos, uint32_t * nal_end_pos) {
    bool nalu_start_found = false;
    bool nalu_end_found = false;
    uint32_t nalu_start_pos = 0;
    uint32_t nalu_end_pos = 0;

    for (uint32_t i=buf_start_pos; i<buf_len-3; i++) {
        if ( buf[i] == 0x00 && buf[i+1] == 0x00
                && buf[i+2] == 0x00 && buf[i+3] == 0x01)
        {
            if ( nalu_start_found == false) {
                nalu_start_pos = i;
                nalu_start_found = true;
            } else if ( nalu_end_found == false ) {
                nalu_end_pos = i - 1;
                nalu_end_found = true;
            }
            i = i + 4;
        }

        if ( i == buf_len
                - 3 /* length of start code - 1 (4-1) */
                - 1 /* last index equal length - 1 */ )
        {
            if ( nalu_start_found == true ) {
                nalu_end_pos = i + 3;
                nalu_end_found = true;
            }
        }

        if ( nalu_start_found == true && nalu_end_found == true ) {
            (* nal_start_pos) = nalu_start_pos;
            (* nal_end_pos) = nalu_end_pos;
            return true;
        }
    }
    return false;
}

bool find_adts_unit(uint8_t * buf, uint32_t buf_len, uint32_t buf_start_pos, uint32_t * adts_start_pos, uint32_t * adts_end_pos) {
    bool adtsu_start_found = false;
    bool adtsu_end_found = false;
    uint32_t adtsu_start_pos = 0;
    uint32_t adtsu_end_pos = 0;

    for (uint32_t i=buf_start_pos; i<buf_len-1; i++) {
        if ( buf[i] == 0xff
                && buf[i+1] == 0xf1)
        {
            if ( adtsu_start_found == false) {
                adtsu_start_pos = i;
                adtsu_start_found = true;
            } else if ( adtsu_end_found == false ) {
                adtsu_end_pos = i - 1;
                adtsu_end_found = true;
            }
            i = i + 2;
        }

        if ( i == buf_len
                - 1 /* length of start code - 1 (2-1)*/
                - 1 /* last index equal length - 1 */ )
        {
            if ( adtsu_start_found == true ) {
                adtsu_end_pos = i + 1;
                adtsu_end_found = true;
            }
        }

        if ( adtsu_start_found == true && adtsu_end_found == true ) {
            (* adts_start_pos) = adtsu_start_pos;
            (* adts_end_pos) = adtsu_end_pos;
            return true;
        }
    }
    return false;
}

