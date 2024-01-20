#include "ECCProcessor.h"

ECCProcessor::ECCProcessor() {
    // Inizializzazione, se necessario
}

void ECCProcessor::encryption(){//const String& message) {
    //Encryption
    //ecc.plaintext=message;
    Serial.println("Plain Text: "+ ecc.plaintext);
    ecc.encrypt();
    Serial.println("Cipher Text: "+ ecc.ciphertext);
    //Signature Generation
    ecc.genSig();
    Serial.println("Signature:"+String(ecc.Sig[0])+","+String(ecc.Sig[1]));
}

String ECCProcessor::plainText(const String& message)
{
    ecc.plaintext = message;
    return message;
}

String ECCProcessor::generateSignature(const String& message) {
    //ecc.plaintext = message;
    ecc.genSig();
    return String(ecc.Sig[0]) + "," + String(ecc.Sig[1]);
}

bool ECCProcessor::verifySignature(const String& message, const String& signature) {
    // Questo dipende da come tinyECC gestisce la verifica delle firme
    // e come vuoi passare la firma alla funzione (ad esempio, come String o come due componenti separate)
    ecc.plaintext = message;
    // ecc.signature = ...; // Devi impostare la firma qui
    return ecc.verifySig();
}

ECCProcessor eccProcessor;