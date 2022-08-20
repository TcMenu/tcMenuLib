
#ifndef TCMENU_TCMENUHTTPREQUESTPROCESSOR_H
#define TCMENU_TCMENUHTTPREQUESTPROCESSOR_H

#include "TransportNetworkDriver.h"
#include <TaskManagerIO.h>

#define WS_SERVER_NAME "tccWS"
#define WS_TEXT_RESPONSE_NOT_FOUND "Not found"
#define WS_INT_RESPONSE_NOT_FOUND 404
#define WS_INT_RESPONSE_OK 200
#define WS_TEXT_RESPONSE_OK "OK"
#define WS_INT_RESPONSE_INT_ERR 500
#define WS_CODE_CHANGING_PROTOCOL 101

#define WS_FORCE_CLOSE_ON_END true

namespace tcremote {

    class AbstractWebSocketTcMenuTransport;

    /**
     * Enumerates the various types of request that this server can handle, or none/error for the case that we cannot
     * process a given request.
     */
    enum WebServerMethod { GET, POST, WS_UPGRADE, REQ_NONE, REQ_ERROR };

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

    class TcMenuWebServerTransport;

    /**
     * The HTTP processor is responsible for actually parsing data from a HTTP request, it can read data from a socket
     * asynchronously allowing other tasks to run while the read is completed. It allows other code to run while the
     * reads are taking place by calling yield on task manager. It separates processing the request line from the
     * headers so as to require as little memory storage as possible. With the current approach only one buffer is
     * needed large enough to fit one single parameter (either path or header value) at once.
     */
    class HttpProcessor {
    private:
        TcMenuWebServerTransport *transport;
        unsigned long millisStart;
        bool protocolError = false;
    public:
        explicit HttpProcessor(TcMenuWebServerTransport* transport) : transport(transport), millisStart(0) {}

        void reset();

        bool readWordUntilTrim(char *buffer, size_t bufferSize, bool skipSeparator = false);

        char readCharFromTransport();

        WebServerMethod processRequest(char *buffer, size_t bufferSize);

        WebServerHeader processHeader(char *buffer, size_t bufferSize);

        void tick();

        bool isProtocolError() const {return protocolError;}
    };

    class TcMenuLightweightWebServer;

    /**
     * A Webserver Response object is responsible for parsing the request line and header data out of an incoming request
     * (potentially iteratively if there's more than one request) and then providing the means to respond to the request
     * in the usual manner (eg sending headers, then data, then calling end, once end() is called, you can check if there
     * is another request within the same transport.
     */
    class WebServerResponse : public Executable {
    public:
        enum WSRMode { NOT_IN_USE, TRANSPORT_ASSIGNED, READING_HEADERS, PREPARING_HEADER, PREPARING_CONTENT, WEBSOCKET_BUSY };
        enum WSRContentType { PLAIN_TEXT, HTML_TEXT, PNG_IMAGE, JPG_IMAGE, WEBP_IMAGE, JSON_TEXT, TEXT_CSS, JAVASCRIPT, IMG_ICON };
        enum WSRConnectionType { KEEP_REQ_OPEN, CLOSE_AFTER_RESPONSE, WEB_SOCKET };
    private:
        TcMenuLightweightWebServer* webServer;
        WebServerMethod method;
        TcMenuWebServerTransport* transport;
        HttpProcessor processor;
        enum WSRMode mode;
        WSRConnectionType connectionType;
        uint8_t webSocketSha1KeyToRespond[20];
        taskid_t scheduledTaskId = TASKMGR_INVALIDID;
    public:
        WebServerResponse(TcMenuLightweightWebServer* webServer, TcMenuWebServerTransport* tx, WSRConnectionType conType);
        void init();
        void stop();
        void serviceClient(socket_t sock);
        bool processHeaders();

        WSRMode getMode() { return mode; }
        void setMode(WSRMode newMode) { mode = newMode; }

        /**
         * Starts a standard response to the server, IE 200 and OK
         */
        void startHeader() { startHeader(WS_INT_RESPONSE_OK, WS_TEXT_RESPONSE_OK); }

        /**
         * Starts a header given a HTTP response code and the text for that code
         * @param code the response code
         * @param textualInfo the text for that response
         */
        void startHeader(int code, const char* textualInfo);

        /**
         * Set a header onto the response
         * @param header the header type
         * @param headerValue the value for the header
         */
        void setHeader(WebServerHeader header, const char* headerValue);

        /**
         * Tells this request handler that the request we are processing is a websocket, usually called during
         * header processing, this automatically starts the header and adds most web socket headers. Including the
         * base64 sec header.
         */
        void turnRequestIntoWebSocket();

        /**
         * Called during header processing to send the content type and length of the data, this should always be
         * called before starting data transmission
         * @param contentType one of the standard content types
         * @param len the length of the data to send
         */
        void contentInfo(WSRContentType contentType, size_t len);

        /**
         * Tells the HTTP layer that the header is complete and we will start the data response, can be omitted and
         * the first call to send.. will call this.
         */
        void startData();

        /**
         * Sends data to the HTTP client using the buffer provided, where the data is in program memory. This will block
         * until the bytes are sent, but it will yield to taskmanager frequently so other tasks still run.
         * @param startingLocation the starting location
         * @param numBytes the number of bytes to send
         * @return true if the bytes were sent.
         */
        bool send_P(const char* startingLocation, size_t numBytes) { return send_P((uint8_t*) startingLocation, numBytes);}

        /**
         * Sends data to the HTTP client using the buffer provided, where the data is in program memory. This will block
         * until the bytes are sent, but it will yield to taskmanager frequently so other tasks still run.
         * @param startingLocation the starting location
         * @param numBytes the number of bytes to send
         * @return true if the bytes were sent.
         */
        bool send_P(const uint8_t* startingLocation, size_t numBytes);

        /**
         * Sends data to the HTTP client using the buffer provided, if the memory is constant, in many cases it
         * optimises the amount of memory needed in the network layer. This will block until the bytes are sent,
         * but it will yield to taskmanager frequently so other tasks still run.
         * @param startingLocation the starting location
         * @param numBytes the number of bytes to send
         * @param memIsConst indicates if the memory is constant and doesn't need copying.
         * @return true if the bytes were sent.
         */
        bool send(const char* startingLocation, size_t numBytes, bool memIsConst = false) { return send((uint8_t*)startingLocation, numBytes, memIsConst);}

        /**
         * Sends data to the HTTP client using the buffer provided, if the memory is constant, in many cases it
         * optimises the amount of memory needed in the network layer. This will block until the bytes are sent,
         * but it will yield to taskmanager frequently so other tasks still run.
         * @param startingLocation the starting location
         * @param numBytes the number of bytes to send
         * @param memIsConst indicates if the memory is constant and doesn't need copying.
         * @return true if the bytes were sent.
         */
        bool send(const uint8_t* startingLocation, size_t numBytes, bool memIsConst = false);

        /**
         * Use this to end the request after the data and headers has been sent. Can be omitted in most cases and the
         * transport will do this automatically
         */
        void end();

        /**
         * Send an error back to the client in the event we couldn't process the request properly
         * @param code the HTTP error to send back
         */
        void sendError(int code);

        /**
         * Close the HTTP connection, usually called by end or sendError, but this is there for special cases.
         */
        void closeConnection();

        /**
         * Indicates if the request should close directly after the first HTTP request is processed.
         * @return
         */
        bool isInSingleShotMode() { return connectionType == CLOSE_AFTER_RESPONSE; }

        /**
         * Get the http method of the present request, this can also be used to check if we have a websocket upgrade
         * request in progress too.
         * @return the http method.
         */
        WebServerMethod getMethod() { return method; }

        /**
         * @return the underlying transport for this request.
         */
        TcMenuWebServerTransport* getTransport() { return transport; }

        /**
         * @return true if there has been an error during processing, otherwise false.
         */
        bool hasErrorOccurred();

        /**
         * This is the executable implementation, it is used to service the request when active.
         */
        void exec() override;
    };
}

#endif //TCMENU_TCMENUHTTPREQUESTPROCESSOR_H
