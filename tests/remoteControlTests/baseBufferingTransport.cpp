#include <AUnit.h>
#include <tcMenu.h>
#include <remote/BaseBufferedRemoteTransport.h>
#include <SimpleCollections.h>

using namespace aunit;
using namespace tcremote;

class UnitTestBufferedTransport : public BaseBufferedRemoteTransport {
private:
    bool bufferComplete;
public:
    int fillReadBuffer(uint8_t *dataBuffer, int maxSize) override {
        return 1;
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