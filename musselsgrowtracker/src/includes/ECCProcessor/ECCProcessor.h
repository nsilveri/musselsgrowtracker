#ifndef MYECC_H
#define MYECC_H

//#include "nanoEccLib\ecc.h"
#include <Arduino.h>
#include <cstddef>
#include <cstdint> 
//#include "nanoEccLib\libs\ecdsa.h" 
//#include "sha256.h"
//#include <AESLib.h>

class MyECC {
public:

    static String generateSHA256Hash(String data);
    int signature_func();
    
    //static void printSHA256Hash(const uint8_t *hash);
    static int my_rng_function(uint8_t *dest, unsigned size);
    void printHex(const uint8_t* data, size_t length);

};

extern MyECC eccProcessor;

#endif // ECC_PROCESSOR_H
