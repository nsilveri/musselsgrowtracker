#include "ECCProcessor.h"
#include <Arduino.h>
#include "sha256.h"

#include <Arduino.h>
#include <uECC.h>

#include <stdio.h>
#include <string.h>
#include "..\utilities\utilities.h"
#include <stdarg.h>
#include <stm32l0xx.h>
#include "lib\ecc.h"
#include "stm32l0_random.h"

void MyECC::p(char *fmt, ... ){
    char tmp[128];
    va_list args;
    va_start (args, fmt );
    vsnprintf(tmp, 128, fmt, args);
    va_end (args);
    Serial.print(tmp);
}

void MyECC::dump(char *text, uint8_t *d) {
    int i;
    p("%-20s", text);
    for (i = 0; i < NUM_ECC_DIGITS; ++i)
        p("%02x", d[NUM_ECC_DIGITS-i-1]);
    Serial.print("\n");
}

uint8_t* MyECC::get_msgSigned() {
    return msgSigned;
}

void MyECC::printHex(const uint8_t* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    if (data[i] < 16) {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
  }
  Serial.println();
}

size_t MyECC::get_msgSignedSize() {
    return 64;
}

void MyECC::sign_message(uint8_t *hash_to_sign) {
    unsigned long then, now;

    if (ECC_CURVE == secp256r1) {
        uint8_t private_0[NUM_ECC_DIGITS] = {0x3c,0x1c,0x76,0x13,0x0d,0x17,0xd9,0x3b,0x55,0xea,0x41,0x74,0xd5,0xc2,0xe1,0x21,0x5a,0x44,0xda,0x89,0xc7,0x61,0x7e,0xcb,0x5d,0xb5,0x05,0x42,0x29,0x14,0x28,0x32};
        
        uint8_t publicKey[64] = {0xd6,0x50,0x54,0x21,0xb3,0x9a,0xa3,0x9c,0x72,0xd1,0x96,0x8a,0x2b,0xe6,0xee,0x3d,0xa1,0x0d,0x89,0x00,0x9a,0x96,0xc7,0x2c,0xfd,0xe9,0x5f,0xde,0x1a,0x4f,0x73,0x9b,0xd2,0x55,0x85,0x83,0x53,0x13,0xd5,0xee,0xe7,0xf4,0xe0,0xc4,0x95,0x23,0x13,0xb6,0xf9,0x81,0x00,0xc4,0xe0,0x4a,0x3a,0x35,0x69,0x08,0x8f,0xd7,0xe3,0x6d,0x93,0xaf};
        
        EccPoint public_0 = {
            {},
            {}
        };

        uint8_t private_2[NUM_ECC_DIGITS];
        EccPoint public_2;
        then = millis();

        Serial.print("ecdsa");
        //HASH PIPPO
        //uint8_t hash_to_sign[NUM_ECC_DIGITS] = {0xa2,0x24,0x2e,0xad,0x55,0xc9,0x4c,0x3d,0xeb,0x7c,0xf2,0x34,0x0b,0xfe,0xf9,0xd5,0xbc,0xac,0xa2,0x2d,0xfe,0x66,0xe6,0x46,0x74,0x5e,0xe4,0x37,0x1c,0x63,0x3f,0xc8};
        uint8_t p_random[32];

        Serial.print("\nPrivate key: ");
            for (int i = 0; i < NUM_ECC_DIGITS; ++i) {
                p("%02x", private_0[i]);
            }
        stm32l0_random(p_random, sizeof(p_random));
        ecc_make_key(&public_0, private_0, NULL);

        dump("\nPublic key x:", public_0.x);
        dump("\nPublic key y:", public_0.y);

        Serial.print("\nPublic_key is ");
        if(ecc_valid_public_key(&public_0)) {
            Serial.println("valid!");
        }else Serial.println("invalid!");



        then = millis();

        uint8_t signature_r[NUM_ECC_DIGITS];
        uint8_t signature_s[NUM_ECC_DIGITS];

        if(ecdsa_sign(signature_r, signature_s, private_0, p_random, hash_to_sign)) {
            dump("\nSign r:", signature_r);
            Serial.println("Size r: " + String(sizeof(signature_r)));
            dump("Sign s:", signature_s);
            Serial.println("Size s: " + String(sizeof(signature_s)));

            memcpy(msgSigned, signature_r, NUM_ECC_DIGITS);
            memcpy(msgSigned + NUM_ECC_DIGITS, signature_s, NUM_ECC_DIGITS);

            Serial.print("\nSigned message: ");
            for (int i = 0; i < sizeof(msgSigned); ++i) {
                p("%02x", msgSigned[i]);
            }
            Serial.print("\n\n");
        } else {
            Serial.print("Sign error.\n\n");
        }

        now = millis();
        p(" %u ms\n", now-then);

        // Sign check
            if(ecdsa_verify(&public_0, hash_to_sign, signature_r, signature_s)) {
                Serial.print("Sign verify: SUCCESS\n");
            } else {
                Serial.print("Sign verify: FAILED\n");
            }

        Serial.println("======================================================");
    }
}

void MyECC::generateSHA256Hash(String data, uint8_t* hash_out) {
    if (!hash_out) return;

    uint8_t data_buffer[data.length()];
    for (size_t i = 0; i < data.length(); i++) {
        data_buffer[i] = (uint8_t)data.charAt(i);
    }

    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data_buffer, data.length());
    sha256_final(&ctx, hash_out);
}

MyECC eccProcessor;