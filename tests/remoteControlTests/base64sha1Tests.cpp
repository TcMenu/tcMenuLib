
// Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
// This product is licensed under an Apache license, see the LICENSE file in the top-level directory.

#include <AUnit.h>
#include <tcMenu.h>
#include <remote/BaseRemoteComponents.h>
#include <SimpleCollections.h>
#include "remote/TcMenuWebServer.h"

using namespace aunit;
using namespace tcremote;

namespace tc_b64 {
    int base64(const uint8_t *data, int dataSize, uint8_t *buffer, int bufferSize);
}

extern "C" {
int sha1digest(uint8_t *hexDigest, const uint8_t *data, size_t databytes);
}

/// aGVs bG8g d29y bGQ=
/// aGVs bG8g d29y bGQA␁
test(testThatBase64WorksProperly) {
    char szBase64[32];
    tc_b64::base64((const uint8_t*)"hello world!", 12, (uint8_t*)szBase64, sizeof szBase64);
    assertEqual("aGVsbG8gd29ybGQh", szBase64);

    tc_b64::base64((const uint8_t*)"hello world123", 14, (uint8_t*)szBase64, sizeof szBase64);
    assertEqual("aGVsbG8gd29ybGQxMjM=", szBase64);

    tc_b64::base64((const uint8_t*)"Abc", 3, (uint8_t*)szBase64, sizeof szBase64);
    assertEqual("QWJj", szBase64);

    tc_b64::base64((const uint8_t*)"De", 2, (uint8_t*)szBase64, sizeof szBase64);
    assertEqual("RGU=", szBase64);

    tc_b64::base64((const uint8_t*)"R", 1, (uint8_t*)szBase64, sizeof szBase64);
    assertEqual("Ug==", szBase64);
}

const uint8_t expectedSha1[] = { 0x7C, 0x74, 0xC4, 0xB5, 0xBE, 0x9E, 0x30, 0x57, 0xA9,
                                 0xDE, 0xD5, 0x94, 0xD7, 0x59,0x99, 0x21, 0x6B,
                                 0xC3, 0x53, 0x39};

test(testThatSha1WorksProperly) {
    uint8_t sha1Output[20];
    const char szInput[] = "This is a test SHA1 string";
    sha1digest(sha1Output, (const uint8_t*)szInput, strlen(szInput));

    for(size_t i=0; i<sizeof(expectedSha1); i++) {
        assertEqual(expectedSha1[i], sha1Output[i]);
    }
}
