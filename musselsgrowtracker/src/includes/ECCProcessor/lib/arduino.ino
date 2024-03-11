#include "ecc.h"
#include <stdarg.h>
#include <stm32l0xx.h>
#include <stm32l0_random.h>

void p(char *fmt, ... ){
    char tmp[128];
    va_list args;
    va_start (args, fmt );
    vsnprintf(tmp, 128, fmt, args);
    va_end (args);
    Serial.print(tmp);
}

void dump(char *text, uint8_t *d) {
    int i;
    p("%-20s", text);
    for (i = 0; i < NUM_ECC_DIGITS; ++i)
        p("%02x ", d[NUM_ECC_DIGITS-i-1]);
    Serial.print("\n");
    
    p("%-20s", text);
    for (i = 0; i < NUM_ECC_DIGITS; ++i)
        p("%02x", d[NUM_ECC_DIGITS-i-1]);
    Serial.print("\n");
}

void sign_message(uint8_t *hash_to_sign) {
    unsigned long then, now;

    if (ECC_CURVE == secp256r1) {
        // Example private and public keys for secp256r1 (P-256)
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

        Serial.print("\nChiave privata: ");
            for (int i = 0; i < NUM_ECC_DIGITS; ++i) {
                p("%02x", private_0[i]);
            }
        stm32l0_random(p_random, sizeof(p_random));
        //GENERAZIONE AUTOMATICA CHIAVI PUBBLICHE
        ecc_make_key(&public_0, private_0, NULL);

        dump("\nChiave pubblica x:", public_0.x);
        dump("\nChiave pubblica y:", public_0.y);

        Serial.print("\nPublic_key is ");
        if(ecc_valid_public_key(&public_0)) {
            Serial.println("valid!");
        }else Serial.println("invalid!");



        then = millis();

        uint8_t signature_r[NUM_ECC_DIGITS];
        uint8_t signature_s[NUM_ECC_DIGITS];

        if(ecdsa_sign(signature_r, signature_s, private_0, p_random, hash_to_sign)) {
            dump("\nFirma r:", signature_r);
            Serial.println("Size: " + String(sizeof(signature_r)));
            dump("Firma s:", signature_s);
            Serial.println("Size: " + String(sizeof(signature_s)));

            // Stampa dell'intero messaggio firmato
            Serial.print("\nMessaggio Firmato: ");
            for (int i = 0; i < NUM_ECC_DIGITS; ++i) {
                p("%02x", signature_r[i]);
            }
            for (int i = 0; i < NUM_ECC_DIGITS; ++i) {
                p("%02x", signature_s[i]);
            }
            Serial.print("\n\n");
        } else {
            Serial.print("Errore nella firma\n\n");
        }

        now = millis();
        p(" %u ms\n", now-then);

        // Verifica della firma
            if(ecdsa_verify(&public_0, hash_to_sign, signature_r, signature_s)) {
                Serial.print("Verifica della firma: SUCCESSO\n");
            } else {
                Serial.print("Verifica della firma: FALLITA\n");
            }

        Serial.println("======================================================");
    }
}
