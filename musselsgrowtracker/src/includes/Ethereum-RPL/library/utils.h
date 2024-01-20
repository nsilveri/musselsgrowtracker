/*
 * Copyright (c) 2016-2018 . All Rights Reserved.
 */

#ifndef RLP_UTİLS_H
#define RLP_UTİLS_H

#include <stdint.h>

typedef uint32_t pb_size_t;
typedef uint_least8_t pb_byte_t;

extern int size_of_bytes(int str_len);

extern uint8_t strtohex(char c);

extern int hex2byte_arr(char *buf, int len, uint8_t *out, int outbuf_size);

extern void int8_to_char(uint8_t *buffer, int len, char *out);

#endif //RLP_UTİLS_H
