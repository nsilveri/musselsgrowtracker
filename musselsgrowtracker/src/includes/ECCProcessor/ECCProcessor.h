#ifndef ECC_PROCESSOR_H
#define ECC_PROCESSOR_H

#include <Arduino.h>
#include <tinyECC.h>
//#include <uECC.h>

class ECCProcessor {
public:
    ECCProcessor();
    void encryption();//const String& message);
    String plainText(const String& message);
    String generateSignature(const String& message);
    bool verifySignature(const String& message, const String& signature);

private:
    tinyECC ecc;

};

extern ECCProcessor eccProcessor;

#endif // ECC_PROCESSOR_H
