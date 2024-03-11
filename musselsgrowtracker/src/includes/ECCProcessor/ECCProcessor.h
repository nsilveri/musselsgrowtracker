#ifndef MYECC_H
#define MYECC_H

#include <Arduino.h>
#include <cstddef>
#include <cstdint> 
#include "sha256.h"

class MyECC {
    public:
        void generateSHA256Hash(String data, uint8_t* hash_out);
        void sign_message(uint8_t *hash_to_sign);
        void printHex(const uint8_t* data, size_t length);
        uint8_t* get_msgSigned();
        size_t get_msgSignedSize();
    private:
        void p(char *fmt, ... );
        void dump(char *text, uint8_t *d);
        uint8_t msgSigned[64];
};

extern MyECC eccProcessor;

#endif // ECC_PROCESSOR_H
