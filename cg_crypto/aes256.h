#ifndef __AES256_H__
#define __AES256_H__

#include <stdio.h>
#include <stdlib.h>
#ifndef uint8_t
#define uint8_t unsigned char
#endif

#define GETU32(pt)                                                             \
  (((unsigned int)(pt)[0] << 24) ^ ((unsigned int)(pt)[1] << 16) ^             \
   ((unsigned int)(pt)[2] << 8) ^ ((unsigned int)(pt)[3]))

#define PUTU32(ct, st)                                                         \
  {                                                                            \
    (ct)[0] = (unsigned char)((st) >> 24);                                     \
    (ct)[1] = (unsigned char)((st) >> 16);                                     \
    (ct)[2] = (unsigned char)((st) >> 8);                                      \
    (ct)[3] = (unsigned char)(st);                                             \
  }

typedef struct {
  uint8_t key[32];
  uint8_t enckey[32];
  uint8_t deckey[32];
  unsigned int iv[4];

} aes256_context;

void aes256_init(aes256_context *, uint8_t * /* key */, unsigned int iv);
void aes256_done(aes256_context *);
void aes256_encrypt_cbc(aes256_context *, uint8_t * /* plaintext */);
void aes256_decrypt_cbc(aes256_context *, uint8_t * /* cipertext */);
void aes256_encrypt(uint8_t *buf, unsigned int lBuf, uint8_t *key,
                    unsigned int iv);
void aes256_decrypt(uint8_t *buf, unsigned int lBuf, uint8_t *key,
                    unsigned int iv);

#define F(x) (((x) << 1) ^ ((((x) >> 7) & 1) * 0x1b))
#define FD(x) (((x) >> 1) ^ (((x)&1) ? 0x8d : 0))

#endif
