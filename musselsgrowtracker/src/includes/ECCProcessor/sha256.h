#ifndef SHA256_H
#define SHA256_H

#include <Arduino.h>

#define SHA256_BLOCK_SIZE 32
#define SHA256_HASH_LENGTH 32

typedef struct {
  uint8_t data[64];
  uint32_t datalen;
  unsigned long long bitlen;
  uint32_t state[8];
} SHA256_CTX;

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len);
void sha256_final(SHA256_CTX *ctx, uint8_t hash[]);
String SHA256_string(String data);
uint8_t SHA256_uint8(String data);

#endif // SHA256_H
