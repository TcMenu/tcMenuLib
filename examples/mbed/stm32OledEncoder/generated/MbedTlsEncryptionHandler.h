//
// Created by dave on 10/06/2024.
//

#ifndef TCCLIBSDK_MBEDTLSENCRYPTIONHANDLER_H
#define TCCLIBSDK_MBEDTLSENCRYPTIONHANDLER_H

#include "remote/BaseBufferedRemoteTransport.h"
#include <mbedtls/aes.h>
#include <mbedtls/ctr_drbg.h>
#include "mbedtls/entropy.h"

#ifndef TCMENU_ENTROPY_CUSTOM_DATA
#define TCMENU_ENTROPY_CUSTOM_DATA "TcMenu Custom Entropy Str"
#endif //TCMENU_ENTROPY_CUSTOM_DATA

namespace tcremote {

    class MbedTlsEncryptionHandler : public EncryptionHandler {
    private:
        mbedtls_entropy_context entropy;
        mbedtls_aes_context aes;
        mbedtls_ctr_drbg_context ctr_drbg;
        unsigned char key[32];
        unsigned char ivEnc[16];
        unsigned char ivDec[16];
    public:
        explicit MbedTlsEncryptionHandler(const uint8_t* keyIn);
        void initialise();
        int encryptData(uint8_t *plainText, int bytesIn, uint8_t *buffer, size_t buffLen) override;
        int decryptData(const uint8_t *encoded, int bytesIn, uint8_t *buffer, size_t buffLen) override;
    };

}

#endif //TCCLIBSDK_MBEDTLSENCRYPTIONHANDLER_H
