#include <AUnit.h>
#include <tcMenu.h>
#include <remote/BaseBufferedRemoteTransport.h>
#include <SimpleCollections.h>

using namespace aunit;
using namespace tcremote;

class UnitTestBufferedTransport : public BaseBufferedRemoteTransport {
private:
    bool bufferComplete = false;
    int sequence = 1;
public:
    int fillReadBuffer(uint8_t *dataBuffer, int maxSize) override {
        int toFill = min(maxSize, 11);
        for(int i=0; i<toFill; i++) {
            dataBuffer[i] = sequence++;
        }
        return toFill;
    }

    void flush() override {
        bufferComplete = true;
    }

    bool available() override {
        return true;
    }

    bool connected() override {
        return true;
    }
};

test(testBufferedTransport) {
    fail();
}