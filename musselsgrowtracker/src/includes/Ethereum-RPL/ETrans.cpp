#include <Arduino.h>
#include <stdint.h>
#include "ETrans.h"
//#include "library/utils.h"
//#include "library/RLP.h"
#include <stdlib.h>
#include <string.h>

// Qui vanno tutte le definizioni e implementazioni da utils.cpp
// ...
int size_of_bytes(int str_len) {
    int out_len = (str_len & 1) ? (str_len + 1) / 2 : str_len / 2;
    return out_len;
}

uint8_t strtohex(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 255;
}

int hex2byte_arr(char *buf, int len, uint8_t *out, int outbuf_size) {
    int i = len - 1;
    int out_len = (len & 1) ? (len + 1) / 2 : len / 2;
    int j = out_len - 1;

    if (j > outbuf_size)
        return -1; /* Output buffer is smaller than need */

    while (i >= 0) {
        out[j] = strtohex(buf[i--]);
        if (i >= 0) {
            out[j--] |= strtohex(buf[i--]) << 4;
        }
    }

    return out_len;
}

void int8_to_char(uint8_t *buffer, int len, char *out) {
    const char hex[] = "0123456789abcdef";
    int max = 2 * len;
    int i = 0;
    int j = 0;
    while (j < len) {
        out[i++] = hex[(buffer[j] >> 4) & 0xF];
        out[i++] = hex[buffer[j] & 0xF];
        j++;
    }
    out[i] = '\0';
}

// Qui vanno tutte le definizioni e implementazioni da RLP.cpp
// ...

#define SIZE_THRESHOLD 56
#define OFFSET_SHORT_LIST 0xc0
#define OFFSET_LONG_LIST 0xf7
#define OFFSET_LONG_ITEM  0xb7
#define OFFSET_SHORT_ITEM 0x80

static int wallet_copy_rpl(uint8_t *source, uint8_t *destination, uint8_t size,
                           int copyPos) {
    int ret_val = copyPos;
    if (size != 0) {
        memcpy(source, destination, size);
        ret_val = ret_val + size;
    }
    return ret_val;
}

static int calculateLengOfV(uint32_t v) {
    // 1byte
    if ((v & 0xFF) == v) {
        return 1;
        // 2byte
    } else if ((v & 0xFFFF) == v) {
        return 2;
        // 3byte
    } else if ((v & 0xFFFFFF) == v) {
        return 3;
        // 4byte
    } else if ((v & 0xFFFFFFFF) == v) {
        return 4;
    } else {
        return -1;
    }
}

void wallet_encode_byte(pb_byte_t singleByte, pb_byte_t *new_bytes) {
    if ((singleByte & 0xFF) == 0) {
        new_bytes[0] = (pb_byte_t) OFFSET_SHORT_ITEM;
    } else if ((singleByte & 0xFF) <= 0x7F) {
        new_bytes[0] = (pb_byte_t) singleByte;
    } else {
        new_bytes[0] = (pb_byte_t) (OFFSET_SHORT_ITEM + 1);
        new_bytes[1] = singleByte;
    }
}

void wallet_encode_short(uint16_t singleShort, pb_byte_t *new_bytes) {
    if ((singleShort & 0xFF) == singleShort)
        wallet_encode_byte((pb_byte_t) singleShort, new_bytes);
    else {
        new_bytes[0] = (pb_byte_t) (OFFSET_SHORT_ITEM + 2);
        new_bytes[1] = (singleShort >> 8 & 0xFF);
        new_bytes[2] = (singleShort >> 0 & 0xFF);
    }
}

int wallet_encode_list(EncodeEthereumSignTx *new_msg, EncodeEthereumTxRequest *new_tx,
                       uint64_t *rawTx) {
    uint32_t totalLength = 0;
    uint8_t *data;

    totalLength += new_msg->nonce.size;
    totalLength += new_msg->gas_price.size;
    totalLength += new_msg->gas_limit.size;
    totalLength += new_msg->to.size;
    totalLength += new_msg->value.size;
    totalLength += new_msg->data_initial_chunk.size;

    totalLength += calculateLengOfV(new_tx->signature_v);; //tx->signature_v.size
    totalLength += new_tx->signature_r.size;
    totalLength += new_tx->signature_s.size;

    int copyPos;
    if (totalLength < SIZE_THRESHOLD) {
        data = (uint8_t*)malloc(1 + totalLength);
        data[0] = (int8_t) (OFFSET_SHORT_LIST + totalLength);
        copyPos = 1;
    } else {
        int tmpLength = totalLength;
        uint8_t byteNum = 0;
        while (tmpLength != 0) {
            ++byteNum;
            tmpLength = tmpLength >> 8;
        }
        tmpLength = totalLength;
        uint8_t *lenBytes;
        lenBytes = (uint8_t*)malloc(byteNum);
        int i;
        for (i = 0; i < byteNum; ++i) {
            lenBytes[byteNum - 1 - i] =
                    (uint8_t) ((tmpLength >> (8 * i)) & 0xFF);
        }
        data = (uint8_t*)malloc(1 + byteNum + totalLength);
        data[0] = (uint8_t) (OFFSET_LONG_LIST + byteNum);
        memcpy(data + 1, lenBytes, byteNum);
        copyPos = byteNum + 1;
        free(lenBytes);
    }

    copyPos = wallet_copy_rpl(data + copyPos, new_msg->nonce.bytes,
                              new_msg->nonce.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->gas_price.bytes,
                              new_msg->gas_price.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->gas_limit.bytes,
                              new_msg->gas_limit.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->to.bytes,
                              new_msg->to.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->value.bytes,
                              new_msg->value.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->data_initial_chunk.bytes,
                              new_msg->data_initial_chunk.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, (uint8_t*)&new_tx->signature_v, calculateLengOfV(new_tx->signature_v), copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_tx->signature_r.bytes,
                              new_tx->signature_r.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_tx->signature_s.bytes,
                              new_tx->signature_s.size, copyPos);

    memcpy(rawTx, data, copyPos);
    return copyPos;
}

void wallet_encode_element(pb_byte_t *bytes, pb_size_t size,
                           pb_byte_t *new_bytes, pb_size_t *new_size, bool remove_leading_zeros) {

    pb_byte_t *pbytes;
    pb_size_t psize;


    if (remove_leading_zeros) {
        int leading_count = 0;
        for (int j = 0; j < size; ++j) {
            pb_byte_t singleByte = bytes[j];
            if ((singleByte | 0x00) == 0) {
                leading_count = leading_count + 1;
            } else {
                break;
            }
        }
        if (leading_count > 0) {
            pbytes = (uint8_t*)malloc(size - leading_count);
            memcpy(pbytes, bytes + 1, (size - leading_count));
            psize = (pb_size_t) (size - leading_count);
        } else {
            pbytes = (uint8_t*)malloc(size);
            memcpy(pbytes, bytes, (size));
            psize = size;
        }
    } else {
        pbytes = (uint8_t*)malloc(size);
        memcpy(pbytes, bytes, (size));
        psize = size;
    }

    if (psize == 0) {
        *new_size = 1;
        new_bytes[0] = (pb_byte_t) OFFSET_SHORT_ITEM;
    } else if (psize == 1 && pbytes[0] == 0x00) {
        *new_size = 1;
        new_bytes[0] = pbytes[0];
    } else if (psize == 1 && ((pbytes[0] & 0xFF) == 0)) {
        *new_size = 1;
        new_bytes[0] = pbytes[0];
    } else if (psize == 1 && (pbytes[0] & 0xFF) < 0x80) {
        *new_size = 1;
        new_bytes[0] = pbytes[0];
    } else if (psize < SIZE_THRESHOLD) {
        pb_byte_t length = (pb_byte_t) (OFFSET_SHORT_ITEM + psize);
        new_bytes[0] = length;
        memcpy(new_bytes + 1, pbytes, psize);
        *new_size = psize + 1;
    } else {
        int tmpLength = psize;
        pb_byte_t lengthOfLength = (pb_byte_t) 0;
        while (tmpLength != 0) {
            ++lengthOfLength;
            tmpLength = tmpLength >> 8;
        }
        pb_byte_t *data = (uint8_t*)malloc(1 + lengthOfLength + psize);
        data[0] = (pb_byte_t) (OFFSET_LONG_ITEM + lengthOfLength);
        tmpLength = psize;
        int i;
        for (int i = lengthOfLength; i > 0; --i) {
            data[i] = (pb_byte_t) (tmpLength & 0xFF);
            tmpLength = tmpLength >> 8;
        }
        memcpy(data + 1 + lengthOfLength, pbytes, psize);
        memcpy(new_bytes, data, ((1 + lengthOfLength + psize)));
        *new_size = (1 + lengthOfLength + psize);
        free(pbytes);
        free(data);
    }
}

void wallet_encode_int(uint32_t singleInt, pb_byte_t *new_bytes) {
    if ((singleInt & 0xFF) == singleInt) {
        wallet_encode_byte((pb_byte_t) singleInt, new_bytes);
    } else if ((singleInt & 0xFFFF) == singleInt) {
        wallet_encode_short((uint16_t) singleInt, new_bytes);
    } else if ((singleInt & 0xFFFFFF) == singleInt) {
        new_bytes[0] = (pb_byte_t) (OFFSET_SHORT_ITEM + 3);
        new_bytes[1] = (pb_byte_t) (singleInt >> 16);
        new_bytes[2] = (pb_byte_t) (singleInt >> 8);
        new_bytes[3] = (pb_byte_t) (singleInt);

    } else {
        new_bytes[0] = (pb_byte_t) (OFFSET_SHORT_ITEM + 4);
        new_bytes[1] = (pb_byte_t) (singleInt >> 24);
        new_bytes[2] = (pb_byte_t) (singleInt >> 16);
        new_bytes[3] = (pb_byte_t) (singleInt >> 8);
        new_bytes[4] = (pb_byte_t) (singleInt);
    }
}


//ETH::ETH() {
//    // Costruttore (se necessario)
//}

int ETH::wallet_ethereum_assemble_tx(EthereumSignTx *msg, EthereumSig *tx, uint64_t *rawTx) {
    EncodeEthereumSignTx new_msg;
    EncodeEthereumTxRequest new_tx;
    memset(&new_msg, 0, sizeof(new_msg));
    memset(&new_tx, 0, sizeof(new_tx));

    wallet_encode_element(msg->nonce.bytes, msg->nonce.size,
                          new_msg.nonce.bytes, &(new_msg.nonce.size), false);
    wallet_encode_element(msg->gas_price.bytes, msg->gas_price.size,
                          new_msg.gas_price.bytes, &(new_msg.gas_price.size), false);
    wallet_encode_element(msg->gas_limit.bytes, msg->gas_limit.size,
                          new_msg.gas_limit.bytes, &(new_msg.gas_limit.size), false);
    wallet_encode_element(msg->to.bytes, msg->to.size, new_msg.to.bytes,
                          &(new_msg.to.size), false);
    wallet_encode_element(msg->value.bytes, msg->value.size,
                          new_msg.value.bytes, &(new_msg.value.size), false);
    wallet_encode_element(msg->data_initial_chunk.bytes,
                          msg->data_initial_chunk.size, new_msg.data_initial_chunk.bytes,
                          &(new_msg.data_initial_chunk.size), false);

    // Assuming tx->signature_v is uint32_t and new_tx.signature_v is pb_byte_t array
    uint32_t v = tx->signature_v; // Get the integer value
    pb_byte_t encoded_v[4]; // Create a byte array large enough to hold the encoded integer
    wallet_encode_int(v, encoded_v); // Encode the integer into the byte array
    memcpy(&new_tx.signature_v, encoded_v, sizeof(encoded_v)); // Copy the encoded data
    //wallet_encode_int(tx->signature_v, &(new_tx.signature_v));
    wallet_encode_element(tx->signature_r.bytes, tx->signature_r.size,
                          new_tx.signature_r.bytes, &(new_tx.signature_r.size), true);
    wallet_encode_element(tx->signature_s.bytes, tx->signature_s.size,
                          new_tx.signature_s.bytes, &(new_tx.signature_s.size), true);

    int length = wallet_encode_list(&new_msg, &new_tx, rawTx);
    return length;
}

void ETH::assembleTx(const char *data) {
    static char rawTx[256];
    EthereumSignTx tx;
    EthereumSig signature;
    uint64_t raw_tx_bytes[24];
    const char *nonce = "00";
    const char *gas_price = "4a817c800";
    const char *gas_limit = "5208";
    const char *to = "e0defb92145fef3c3a945637705fafd3aa74a241";
    const char *value = "de0b6b3a7640000";
    //const char *data = "00";
    const char *r = "09ebb6ca057a0535d6186462bc0b465b561c94a295bdb0621fc19208ab149a9c";
    const char *s = "440ffd775ce91a833ab410777204d5341a6f9fa91216a6f3ee2c051fea6a0428";
    uint32_t v = 27;

    tx.nonce.size = size_of_bytes(strlen(nonce));
    hex2byte_arr((char*)nonce, strlen(nonce), tx.nonce.bytes, tx.nonce.size);

    tx.gas_price.size = size_of_bytes(strlen(gas_price));
    hex2byte_arr((char*)gas_price, strlen(gas_price), tx.gas_price.bytes, tx.gas_price.size);

    tx.gas_limit.size = size_of_bytes(strlen(gas_limit));
    hex2byte_arr((char*)gas_limit, strlen(gas_limit), tx.gas_limit.bytes, tx.gas_limit.size);

    tx.to.size = size_of_bytes(strlen(to));
    hex2byte_arr((char*)to, strlen(to), tx.to.bytes, tx.to.size);

    tx.value.size = size_of_bytes(strlen(value));
    hex2byte_arr((char*)value, strlen(value), tx.value.bytes, tx.value.size);

    tx.data_initial_chunk.size = size_of_bytes(strlen(data));
    hex2byte_arr((char*)data, strlen(data), tx.data_initial_chunk.bytes,
                 tx.data_initial_chunk.size);

    signature.signature_v = 27;

    signature.signature_r.size = size_of_bytes(strlen(r));
    hex2byte_arr((char*)r, strlen(r), signature.signature_r.bytes, signature.signature_r.size);

    signature.signature_s.size = size_of_bytes(strlen(s));
    hex2byte_arr((char*)s, strlen(s), signature.signature_s.bytes, signature.signature_s.size);

    int length = wallet_ethereum_assemble_tx(&tx, &signature, raw_tx_bytes);
    int8_to_char((uint8_t *) raw_tx_bytes, length, rawTx);
    Serial.print("raw transaction: ");
    Serial.println(rawTx);
}



ETH EthTr;
