#include "ECCProcessor.h"
#include <Arduino.h>
#include "sha256.h"

#include <Arduino.h>
#include <uECC.h>

#include <stdio.h>
#include <string.h>
//#include <random.h>

void MyECC::printHex(const uint8_t* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    if (data[i] < 16) {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
  }
  Serial.println();
}

int MyECC::signature_func(/*uint8_t hash*/) {
    int i, c;
    uint8_t privateKey[32] = {0x3c,0x1c,0x76,0x13,0x0d,0x17,0xd9,0x3b,0x55,0xea,0x41,0x74,0xd5,0xc2,0xe1,0x21,0x5a,0x44,0xda,0x89,0xc7,0x61,0x7e,0xcb,0x5d,0xb5,0x05,0x42,0x29,0x14,0x28,0x32};
    uint8_t publicKey[64] = {0xd6,0x50,0x54,0x21,0xb3,0x9a,0xa3,0x9c,0x72,0xd1,0x96,0x8a,0x2b,0xe6,0xee,0x3d,0xa1,0x0d,0x89,0x00,0x9a,0x96,0xc7,0x2c,0xfd,0xe9,0x5f,0xde,0x1a,0x4f,0x73,0x9b,0xd2,0x55,0x85,0x83,0x53,0x13,0xd5,0xee,0xe7,0xf4,0xe0,0xc4,0x95,0x23,0x13,0xb6,0xf9,0x81,0x00,0xc4,0xe0,0x4a,0x3a,0x35,0x69,0x08,0x8f,0xd7,0xe3,0x6d,0x93,0xaf};
    uint8_t hash[32] = {0xA2,0x24,0x2E,0xAD,0x55,0xC9,0x4C,0x3D,0xEB,0x7C,0xF2,0x34,0x0B,0xFE,0xF9,0xD5,0xBC,0xAC,0xA2,0x2D,0xFE,0x66,0xE6,0x46,0x74,0x5E,0xE4,0x37,0x1C,0x63,0x3F,0xC8};
    uint8_t sig[64];

    const struct uECC_Curve_t * curves[5];
    int num_curves = 0;
#if uECC_SUPPORTS_secp160r1
    curves[num_curves++] = uECC_secp160r1();
#endif
#if uECC_SUPPORTS_secp192r1
    curves[num_curves++] = uECC_secp192r1();
#endif
#if uECC_SUPPORTS_secp224r1
    curves[num_curves++] = uECC_secp224r1();
#endif
#if uECC_SUPPORTS_secp256r1
    curves[num_curves++] = uECC_secp256r1();
#endif
#if uECC_SUPPORTS_secp256k1
    curves[num_curves++] = uECC_secp256k1();
#endif
    
    Serial.println("\nTesting 256 signatures\n");
    for (c = 3; c < 4; ++c) {
      //Serial.println("curva: %i\n" + String(c));      
      Serial.println("curva: " + String(c));
        //for (i = 0; i < 256; ++i) {
            //printf(".");
            fflush(stdout);

           /*if (!uECC_make_key(publicKey, privateKey, curves[c])) {
                printf("uECC_make_key() failed\n");
                return 1;
            }
            */
            
            //memcpy(hash, publicKey, sizeof(hash));
            int ret = uECC_sign(privateKey, hash, sizeof(hash), sig, curves[i]);

            if (!ret) {
                Serial.println("\nuECC_sign() failed\n");
                return 1;
            }
            Serial.println("\nsignature\n");
            //for(int k = 0; k < 64; k++){
            //  printf("%02x", sig[k]);
            //}
            //for(int k = 0; k < 64; k++){
              //printf("%02x", sig[k]);
            //  Serial.print(sig[k], HEX);
            //}
            char result[129]; // Each byte becomes two characters in hex, plus null terminator
            result[0] = '\0'; // Start with an empty string

            char buffer[3]; // Temporary buffer for each byte
            for (int k = 0; k < 64; k++) {
                snprintf(buffer, sizeof(buffer), "%02x", sig[k]);
                strcat(result, buffer); // Append the hex byte to the result string
            }
            for(int i = 0; i < sizeof(result); i++){
                Serial.print(String(result[i]));
            }
            Serial.println();
            
            Serial.println("\nend signature\n");

            int ret_two = uECC_verify(publicKey, hash, sizeof(hash), sig, curves[c]);
            
            Serial.println("ret: " + String(ret));
            Serial.println("ret_two: " + String(ret_two));

            if (!ret_two) {
                Serial.println("uECC_verify() failed\n");
                return 1;
            }
            if(c == 4 && i == 0){
            Serial.println("\nPublic\n");
            for(int k = 0; k < 64; k++ ) {
              //printf("%02x ", publicKey[k]);
              Serial.print(publicKey[k], HEX);
            }
            Serial.println("\n");
            Serial.println("\nPrivate\n");
            for(int k = 0; k < 32; k++ ) {
              //printf("%02x ", privateKey[k]);
              Serial.print(privateKey[k], HEX);        
              }    
            }
        //}
        Serial.println("\n");
    }
    for (int k = 0; k < 64; k++) {
      Serial.print(sig[k], HEX); // Print each element in hexadecimal
      Serial.print(" "); // Add a space for readability
    }
Serial.println(); // Move to a new line after printing the entire array

    return 0;
}

String MyECC::generateSHA256Hash(String data) {
    //this should return : f6774519d1c7a3389ef327e9c04766b999db8cdfb85d1346c471ee86d65885bc
    //String pin = "this is a test string";
    String sha = SHA256(data);
    //Serial.print(sha);
    return sha;
}

MyECC eccProcessor;