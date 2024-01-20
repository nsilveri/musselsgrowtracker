#ifndef ETRANS_H
#define ETRANS_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

//#include "library\RLP.h"
//#include "library\utils.h"

// Qui vanno tutte le dichiarazioni da utils.h
// ...
typedef uint32_t pb_size_t;
typedef uint_least8_t pb_byte_t;

extern int size_of_bytes(int str_len);

extern uint8_t strtohex(char c);

extern int hex2byte_arr(char *buf, int len, uint8_t *out, int outbuf_size);

extern void int8_to_char(uint8_t *buffer, int len, char *out);

// Qui vanno tutte le dichiarazioni da RLP.h
// ...
#define PB_BYTES_ARRAY_T(n) struct { pb_size_t size; pb_byte_t bytes[n]; }

typedef PB_BYTES_ARRAY_T(32) EthereumSignTx_nonce_t;
typedef PB_BYTES_ARRAY_T(32) EthereumSignTx_gas_price_t;
typedef PB_BYTES_ARRAY_T(32) EthereumSignTx_gas_limit_t;
typedef PB_BYTES_ARRAY_T(20) EthereumSignTx_to_t;
typedef PB_BYTES_ARRAY_T(32) EthereumSignTx_value_t;
typedef PB_BYTES_ARRAY_T(1024) EthereumSignTx_data_initial_chunk_t;

typedef struct _EthereumSignTx {
    EthereumSignTx_nonce_t nonce;
    EthereumSignTx_gas_price_t gas_price;
    EthereumSignTx_gas_limit_t gas_limit;
    EthereumSignTx_to_t to;
    EthereumSignTx_value_t value;
    EthereumSignTx_data_initial_chunk_t data_initial_chunk;
} EthereumSignTx;

typedef PB_BYTES_ARRAY_T(64) EthereumTxRequest_signature_r_t;
typedef PB_BYTES_ARRAY_T(64) EthereumTxRequest_signature_s_t;
typedef struct _EthereumTxRequest {
    uint32_t data_length;
    uint32_t signature_v;
    EthereumTxRequest_signature_r_t signature_r;
    EthereumTxRequest_signature_s_t signature_s;
} EthereumSig;


typedef PB_BYTES_ARRAY_T(33) EncodeEthereumSignTx_nonce_t;
typedef PB_BYTES_ARRAY_T(33) EncodeEthereumSignTx_gas_price_t;
typedef PB_BYTES_ARRAY_T(33) EncodeEthereumSignTx_gas_limit_t;
typedef PB_BYTES_ARRAY_T(21) EncodeEthereumSignTx_to_t;
typedef PB_BYTES_ARRAY_T(33) EncodeEthereumSignTx_value_t;
typedef PB_BYTES_ARRAY_T(1025) EncodeEthereumSignTx_data_initial_chunk_t;
typedef struct _EncodeEthereumSignTx {
    pb_size_t address_n_count;
    uint32_t address_n[8];
    bool has_nonce;
    EncodeEthereumSignTx_nonce_t nonce;
    bool has_gas_price;
    EncodeEthereumSignTx_gas_price_t gas_price;
    bool has_gas_limit;
    EncodeEthereumSignTx_gas_limit_t gas_limit;
    bool has_to;
    EncodeEthereumSignTx_to_t to;
    bool has_value;
    EncodeEthereumSignTx_value_t value;
    bool has_data_initial_chunk;
    EncodeEthereumSignTx_data_initial_chunk_t data_initial_chunk;
    bool has_data_length;
    uint32_t data_length;
    bool has_chain_id;
    uint32_t chain_id;
/* @@protoc_insertion_point(struct:EthereumSignTx) */
} EncodeEthereumSignTx;


typedef PB_BYTES_ARRAY_T(33) EncodeEthereumTxRequest_signature_r_t;
typedef PB_BYTES_ARRAY_T(33) EncodeEthereumTxRequest_signature_s_t;
typedef struct _EncodeEthereumTxRequest {
    bool has_data_length;
    uint32_t data_length;
    bool has_signature_v;
    uint32_t signature_v;
    bool has_signature_r;
    EncodeEthereumTxRequest_signature_r_t signature_r;
    bool has_signature_s;
    EncodeEthereumTxRequest_signature_s_t signature_s;
/* @@protoc_insertion_point(struct:EthereumTxRequest) */
} EncodeEthereumTxRequest;

void wallet_encode_byte(pb_byte_t singleByte, pb_byte_t *new_bytes);

void wallet_encode_short(uint16_t singleShort, pb_byte_t *new_bytes);

int wallet_encode_list(EncodeEthereumSignTx *new_msg, EncodeEthereumTxRequest *new_tx,
                       uint64_t *rawTx);

void wallet_encode_element(pb_byte_t *bytes, pb_size_t size,
                           pb_byte_t *new_bytes, pb_size_t *new_size, bool remove_leading_zeros);

void wallet_encode_int(uint32_t singleInt, pb_byte_t *new_bytes);

class ETH {
public:
    //ETH(); // Costruttore
    int wallet_ethereum_assemble_tx(EthereumSignTx *msg, EthereumSig *tx, uint64_t *rawTx);
    void assembleTx(const char *data);

private:
    // Altre dichiarazioni private se necessarie
};

extern ETH EthTr;

#endif
