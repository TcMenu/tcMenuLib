
#ifndef TCMENU_TCMENUHTTPREQUESTPROCESSOR_H
#define TCMENU_TCMENUHTTPREQUESTPROCESSOR_H

#define WS_SERVER_NAME "tccWS"
#define WS_TEXT_RESPONSE_NOT_FOUND "Not found"
#define WS_INT_RESPONSE_NOT_FOUND 404
#define WS_INT_RESPONSE_OK 200
#define WS_TEXT_RESPONSE_OK "OK"
#define WS_INT_RESPONSE_INT_ERR 500
#define WS_CODE_CHANGING_PROTOCOL 101

namespace tcremote {

    class AbstractWebSocketTcMenuTransport;

    /**
     * Enumerates the various types of request that this server can handle, or none/error for the case that we cannot
     * process a given request.
     */
    enum WebServerMethod { GET, POST, REQ_NONE, REQ_ERROR };

    /**
     * Provides an enumeration of all supported web server headers that we process, any other header will come through as
     * WSH_UNPROCESSED. When calling process request each header will be returned in turn as one of these enumerations,
     * the buffer you provide will be filled with the value of the header on read. For writing, the header enumeration
     * will be turned into text along with the value provided.
     */
    enum WebServerHeader {
        /** for a header that we cannot process, only valid on read */
        WSH_UNPROCESSED,
        /** got the blank line that ends the header */
        WSH_FINISHED,
        /** upgrade header from the server */
        WSH_UPGRADE_TO_WEBSOCKET,
        /** security key header in websocket upgrade request */
        WSH_SEC_WS_KEY,
        /** The date header */
        WSH_DATE,
        /** The server header */
        WSH_SERVER,
        /** Last modified header, can be set to a GMT time in the right format */
        WSH_LAST_MODIFIED,
        /** The content type header, usually used on write internally during startHeader */
        WSH_CONTENT_TYPE,
        /** The content length header, usually used on write internally during startHeader */
        WSH_CONTENT_LENGTH,
        /** The content encoding header, for example, often set to gzip to send compressed files */
        WSH_CONTENT_ENCODING,
        /** Cache control header, usually used on write */
        WSH_CACHE_CONTROL,
        /** Host header */
        WSH_HOST,
        /** User agent header */
        WSH_USER_AGENT,
        /** Accept encoding header */
        WSH_ACCEPT_ENCODING,
        /** Connection header, used in writes during websocket upgrade */
        WSH_CONNECTION,
        /** The response to the sec key in a websocket upgrade */
        WSH_SEC_WS_ACCEPT_KEY,
        /** Indicates a serious error has occurred that cannot be corrected and the transport should close */
        WSH_ERROR
    };

    /**
     * The HTTP processor is responsible for actually parsing data from a HTTP request, it can read data from a socket
     * asynchronously allowing other tasks to run while the read is completed. It allows other code to run while the
     * reads are taking place by calling yield on task manager. It separates processing the request line from the
     * headers so as to require as little memory storage as possible. With the current approach only one buffer is
     * needed large enough to fit one single parameter (either path or header value) at once.
     */
    class HttpProcessor {
    private:
        AbstractWebSocketTcMenuTransport *transport;
        unsigned long millisStart;
        bool protocolError = false;
    public:
        HttpProcessor() : transport(nullptr), millisStart(0) {}

        void setTransport(AbstractWebSocketTcMenuTransport *tx);

        bool readWordUntilTrim(char *buffer, size_t bufferSize, bool skipSeparator = false);

        char readCharFromTransport();

        WebServerMethod processRequest(char *buffer, size_t bufferSize);

        WebServerHeader processHeader(char *buffer, size_t bufferSize);

        void tick();

        bool isProtocolError() const {return protocolError;}
    };

    /**
     * A Webserver Response object is responsible for parsing the request line and header data out of an incoming request
     * (potentially iteratively if there's more than one request) and then providing the means to respond to the request
     * in the usual manner (eg sending headers, then data, then calling end, once end() is called, you can check if there
     * is another request within the same transport.
     */
    class WebServerResponse {
    public:
        enum WSRMode { NOT_IN_USE, TRANSPORT_ASSIGNED, READING_HEADERS, PREPARING_HEADER, PREPARING_CONTENT };
        enum WSRContentType { PLAIN_TEXT, HTML_TEXT, PNG_IMAGE, JPG_IMAGE, WEBP_IMAGE, JSON_TEXT, TEXT_CSS, JAVASCRIPT, IMG_ICON };
    private:
        WebServerMethod method;
        AbstractWebSocketTcMenuTransport* transport;
        HttpProcessor processor;
        enum WSRMode mode;
        bool singleRequestMode;
    public:
        WebServerResponse() : method(GET), transport(nullptr), mode(NOT_IN_USE), singleRequestMode(false) {}
        void setTransport(AbstractWebSocketTcMenuTransport* tx);
        WebServerMethod processRequestLine();
        bool processHeaders();

        WSRMode getMode() { return mode; }
        void setMode(WSRMode newMode) { mode = newMode; }

        void startHeader() { startHeader(WS_INT_RESPONSE_OK, WS_TEXT_RESPONSE_OK); }
        void startHeader(int code, const char* textualInfo);
        void setHeader(WebServerHeader header, const char* headerValue);
        void contentInfo(WSRContentType contentType, size_t len);
        void startData();
        bool send_P(const char* startingLocation, size_t numBytes) { return send_P((uint8_t*) startingLocation, numBytes);}
        bool send(const char* startingLocation, size_t numBytes) { return send((uint8_t*)startingLocation, numBytes);}
        bool send_P(const uint8_t* startingLocation, size_t numBytes);
        bool send(const uint8_t* startingLocation, size_t numBytes);
        void end();

        void closeConnection();

        const char* getLastData();
        WebServerMethod getMethod() { return method; }

        bool hasErrorOccurred();
    };
}

#endif //TCMENU_TCMENUHTTPREQUESTPROCESSOR_H
