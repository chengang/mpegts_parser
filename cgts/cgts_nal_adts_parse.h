#ifndef __CGTS_NAL_ADTS_PARSE_H__
#define __CGTS_NAL_ADTS_PARSE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define CGTS_NAL_HEADER_SIZE    5
#define CGTS_ADTS_HEADER_SIZE   7

#define CGTS_NAL_TYPE_IDR_SLICE     0x05

bool find_nal_unit(uint8_t * buf, uint32_t buf_len, uint32_t buf_start_pos, uint32_t * nal_start_pos, uint32_t * nal_end_pos);
bool find_adts_unit(uint8_t * buf, uint32_t buf_len, uint32_t buf_start_pos, uint32_t * adts_start_pos, uint32_t * adts_end_pos);

#endif
