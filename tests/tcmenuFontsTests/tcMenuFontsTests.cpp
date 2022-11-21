#include <PlatformDetermination.h>
#include <testing/SimpleTest.h>
#include <graphics/

IOLOG_MBED_PORT_IF_NEEDED(USBTX, USBRX)

void setup() {
    IOLOG_START_SERIAL
    startTesting();
}

DEFAULT_TEST_RUNLOOP

test(utfEncodingAsciiExtended) {
    Utf8
}