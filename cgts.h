#ifndef __CGTS_H__
#define __CGTS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "util.h"

bool cgts_pxx_packet_append(struct cgts_context * ct, uint16_t pid, bool is_start, const uint8_t * ts_payload);

bool cgts_sdt_parse(struct cgts_context * ct, const uint8_t * buf);
bool cgts_cat_parse(struct cgts_context * ct, const uint8_t * buf);
bool cgts_pat_parse(struct cgts_context * ct, const uint8_t * buf);
bool cgts_ts_packet_payload_parse(struct cgts_context * ct, const uint8_t * buf);
bool cgts_ts_packet_parse(struct cgts_context * ct, struct cgts_ts_packet * tsp, uint8_t * buf);
bool cgts_analyze_ts_packet(struct cgts_context * ct, uint8_t * buf);

bool cgts_get188_from_file(FILE * fp, uint8_t * buf);
bool cgts_get188(struct cgts_context * context, uint8_t * buf);
void cgts_parse(struct cgts_context * context);

#endif
