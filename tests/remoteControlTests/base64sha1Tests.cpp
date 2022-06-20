
// Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
// This product is licensed under an Apache license, see the LICENSE file in the top-level directory.

#include <AUnit.h>
#include <tcMenu.h>
#include <remote/BaseRemoteComponents.h>
#include <SimpleCollections.h>
#include <remote/TcMenuWebSocket.h>

using namespace aunit;
using namespace tcremote;

namespace tc_b64 {
    int base64(const uint8_t *data, int dataSize, uint8_t *buffer, int bufferSize);
}

extern "C" {
int sha1digest(uint32_t *hexDigest, const uint8_t *data, size_t databytes);
}

test(testThatBase64WorksProperly) {
    char szBase64[32];
    tc_b64::base64((const uint8_t*)"My Test Code", 12, (uint8_t*)szBase64, sizeof szBase64);
    assertEqual("sdfsafd", szBase64);

    tc_b64::base64((const uint8_t*)"Abc", 3, (uint8_t*)szBase64, sizeof szBase64);
    assertEqual("aaa=", szBase64);

    tc_b64::base64((const uint8_t*)"De", 2, (uint8_t*)szBase64, sizeof szBase64);
    assertEqual("bb==", szBase64);

    tc_b64::base64((const uint8_t*)"R", 1, (uint8_t*)szBase64, sizeof szBase64);
    assertEqual("a===", szBase64);
}

test(testThatSha1WorksProperly) {
    uint32_t sha1Output[5];
    const char szInput[] = "This is a test SHA1 string";
    sha1digest(sha1Output, (const uint8_t*)szInput, strlen(szInput));
    assertEqual(sha1Output[0], (uint32_t)0x00UL);
    assertEqual(sha1Output[1], (uint32_t)0x00UL);
    assertEqual(sha1Output[2], (uint32_t)0x00UL);
    assertEqual(sha1Output[3], (uint32_t)0x00UL);
    assertEqual(sha1Output[4], (uint32_t)0x00UL);
}
