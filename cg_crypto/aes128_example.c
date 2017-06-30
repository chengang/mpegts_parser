#include <stdio.h>
#include "aes128.h"

void print_hex(uint8_t * buf, int len) {
  int i;
  for(i=0;i<len;i++) {
    fprintf(stdout, " %02x", buf[i]);
    if (i % 8 == 7) {
      fprintf(stdout, " ");
    }
    if (i % 16 == 15) {
      fprintf(stdout, "\n");
    }
  }
  fprintf(stdout, "\n");
}

int main() {
    uint8_t in[20] = {
         0x01, 0x01, 0x01, 0x01
        ,0x01, 0x01, 0x01, 0x01
        ,0x01, 0x01, 0x01, 0x01
        ,0x01, 0x01, 0x01, 0x01
        ,0x01, 0x01, 0x01, 0x01
    };
    uint8_t out[32] = {
         0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00

        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
    };

    uint8_t out2[32] = {
         0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00

        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
    };

    uint8_t key[16] = {
         0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
    };

    uint8_t iv[16] = {
         0x00, 0x01, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
        ,0x00, 0x00, 0x00, 0x00
    };

    int len = 0;
	print_hex(in, 20);
	print_hex(out, 32);
    len = AES128_CBC_encrypt_buffer(out, in, 20, key, iv);
    printf("[%d]\n", len);
	print_hex(out, 32);

	print_hex(out2, 32);
    len = AES128_CBC_decrypt_buffer(out2, out, 32, key, iv);
    printf("[%d]\n", len);
	print_hex(out2, 32);
    return 0;
}
