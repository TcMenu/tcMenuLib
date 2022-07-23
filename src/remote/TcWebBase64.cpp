/**
 * When a websocket is upgraded from HTTP to WS protocol, it must answer with a security key, this is made up of the
 * key provided, this UUID, turned into a sha1 and base64 encoded.
 */

#include <PlatformDetermination.h>

namespace tc_b64 {
    /**
     * A lightweight base 64 facility that takes in some data and writes out base 64 into the buffer
     * @param data input data
     * @param dataSize input data size
     * @param buffer the buffer to write to
     * @param bufferSize buffer size
     * @return the number of bytes written.
     */
    int base64(const uint8_t *data, int dataSize, uint8_t *buffer, int bufferSize);
}

namespace tc_b64 {
    const char *b64Dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    void innerBase64(const uint8_t *data, int size, uint8_t *buffer) {
        if (size == 3) {
            buffer[0] = b64Dictionary[data[0] >> 2];
            buffer[1] = b64Dictionary[(data[0] & 0x3) << 4 | (data[1] >> 4)];
            buffer[2] = b64Dictionary[(data[1] & 0x0F) << 2 | (data[2] >> 6)];
            buffer[3] = b64Dictionary[data[2] & 0x3F];
        } else if (size == 2) {
            buffer[0] = b64Dictionary[data[0] >> 2];
            buffer[1] = b64Dictionary[(data[0] & 0x3) << 4 | (data[1] >> 4)];
            buffer[2] = b64Dictionary[(data[1] & 0x0F) << 2];
            buffer[3] = '=';
        } else if (size == 1) {
            buffer[0] = b64Dictionary[data[0] >> 2];
            buffer[1] = b64Dictionary[(data[0] & 0x3) << 4];
            buffer[2] = '=';
            buffer[3] = '=';
        }
    }

    int base64(const uint8_t *data, int dataSize, uint8_t *buffer, int bufferSize) {
        // If we get here we've got enough space to do the encoding

        int writtenBytes = 0;
        // Break the input into 3-byte chunks and process each of them
        int i;
        for (i = 0; i < dataSize / 3; i++) {
            writtenBytes += 4;
            if(writtenBytes >= bufferSize) return -1;
            innerBase64(&data[i * 3], 3, &buffer[i * 4]);
        }
        if (dataSize % 3 > 0) {
            writtenBytes += 4;
            // It doesn't fit neatly into a 3-byte chunk, so process what's left
            innerBase64(&data[i * 3], dataSize % 3, &buffer[i * 4]);
        }

        if(writtenBytes < bufferSize) {
            buffer[writtenBytes] = 0;
        }
        return writtenBytes;
    }
}

