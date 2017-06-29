#ifndef __SHA256_H__
#define __SHA256_H__

#include <stdint.h>
typedef struct {
    uint32_t buf[16];
    uint32_t hash[8];
    uint32_t len[2];
} sha256_context;

void sha256_init(sha256_context *);
void sha256_hash(sha256_context *, uint8_t * /* data */, uint32_t /* len */);
void sha256_done(sha256_context *, uint8_t * /* hash */);
void sha256(unsigned char * data, unsigned int data_len, unsigned char * hash);

#endif
