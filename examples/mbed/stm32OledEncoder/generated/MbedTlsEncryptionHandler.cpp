#include "MbedTlsEncryptionHandler.h"

using namespace tcremote;

MbedTlsEncryptionHandler::MbedTlsEncryptionHandler(const uint8_t* keyIn) {
    memcpy(key, keyIn, sizeof key);
    memset(ivDec, 0, sizeof ivDec);
    memset(ivEnc, 0, sizeof ivEnc);
}

void MbedTlsEncryptionHandler::initialise() {
    // initialise random number generation and aes.
    mbedtls_aes_init(&aes);
    mbedtls_entropy_init( &entropy );
    mbedtls_ctr_drbg_init( &ctr_drbg );

    // create a reasonably random seed, improved if you define TCMENU_ENTROPY_CUSTOM_DATA yourself
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                          (unsigned char *) TCMENU_ENTROPY_CUSTOM_DATA,
                          strlen( TCMENU_ENTROPY_CUSTOM_DATA ));

    // randomize the initialisation vectors
    mbedtls_ctr_drbg_random(&ctr_drbg, ivDec, sizeof ivDec);
    mbedtls_ctr_drbg_random(&ctr_drbg, ivEnc, sizeof ivEnc);
}

size_t roundToNearest(int in) {
    // we need to send in increments of 16 bytes, so we autofill to that boundary.
    auto remainder = in % 16;
    if(remainder == 0) return in;
    return in + (16-remainder);
}

int MbedTlsEncryptionHandler::encryptData(uint8_t *plainText, int bytesIn, uint8_t *buffer, size_t buffLen) {
    mbedtls_aes_setkey_enc(&aes, key, 256);
    // find the nearest 16 byte boundary that is larger than bytesIn.
    size_t totalIn = roundToNearest(bytesIn);
    // ensure the buffers are big enough and then fill the end of the buffer with zeros.
    if(totalIn + 16 >= buffLen) return 0;
    for(size_t i=bytesIn; i<totalIn;i++) {
        plainText[i] = 0;
    }
    // then encrypt the data
    memcpy(buffer, ivEnc, 16);
    bool ok = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, totalIn, ivEnc, plainText, &buffer[16]) == 0;
    return ok ? (int)totalIn : 0;

}

int MbedTlsEncryptionHandler::decryptData(const uint8_t *encoded, int bytesIn, uint8_t *buffer, size_t buffLen) {
    if(bytesIn < 32 || buffLen < bytesIn - 16) return 0;
    memcpy(ivDec, encoded, 16);
    mbedtls_aes_setkey_dec(&aes, key, 256);
    bool ok = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, bytesIn, ivDec, &encoded[16], buffer) == 0;
    return ok ? bytesIn : 0;
}
