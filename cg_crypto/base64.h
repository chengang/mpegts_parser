#ifndef __BASE64_H__
#define __BASE64_H__

#define BASE64_PAD '='
#define BASE64DE_FIRST '+'
#define BASE64DE_LAST 'z'

/* BASE 64 encode table */
static const char base64en[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
};

/* ASCII order for BASE 64 decode, -1 in unused character */
static const signed char base64de[] = {
    /* '+', ',', '-', '.', '/', '0', '1', '2', */ 
        62,  -1,  -1,  -1,  63,  52,  53,  54,

    /* '3', '4', '5', '6', '7', '8', '9', ':', */
        55,  56,  57,  58,  59,  60,  61,  -1,

    /* ';', '<', '=', '>', '?', '@', 'A', 'B', */
        -1,  -1,  -1,  -1,  -1,  -1,   0,   1, 

    /* 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', */
         2,   3,   4,   5,   6,   7,   8,   9,

    /* 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', */ 
        10,  11,  12,  13,  14,  15,  16,  17,

    /* 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', */
        18,  19,  20,  21,  22,  23,  24,  25,

    /* '[', '\', ']', '^', '_', '`', 'a', 'b', */ 
        -1,  -1,  -1,  -1,  -1,  -1,  26,  27,

    /* 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', */ 
        28,  29,  30,  31,  32,  33,  34,  35,

    /* 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', */
        36,  37,  38,  39,  40,  41,  42,  43,

    /* 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', */
        44,  45,  46,  47,  48,  49,  50,  51,
};

int base64_encode(const unsigned char *in, unsigned int inlen, char *out);

int base64_decode(const char *in, unsigned int inlen, unsigned char *out);

#endif
