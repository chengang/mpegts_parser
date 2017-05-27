#ifndef __CGTS_H__
#define __CGTS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "util.h"

bool cgts_get188(struct cgts_context * context, uint8_t * buf);
void cgts_parse(struct cgts_context * context);

#endif
