/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * Ethernet remote capability plugin. This file is a plugin file and should not be directly edited,
 * it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 */

#ifndef TCMENU_MBEDETHERNETTRANSPORT_H
#define TCMENU_MBEDETHERNETTRANSPORT_H

#include <mbed.h>
#include <EthernetInterface.h>
#include <TCPSocket.h>

#include <RemoteConnector.h>
#include <TaskManager.h>
#include <remote/BaseRemoteComponents.h>

// We try and package up writes to avoid writing out single messages, there is a task
// that runs that will flush transports at this interval, around 7 times a second.
#define WRITE_DELAY 142

namespace tcremote {

    /**
     * An mbed implementation of tag val transport that works over a socket connection
     */
    class MBedEthernetTransport : public TagValueTransport, public Executable {
    private:
        char writeBuf[128];
        char readBuf[128];
        size_t readPos;
        size_t lastReadAmt;
        size_t writePos;
        InternetSocket *socket{};
        bool isOpen;
    public:
        MBedEthernetTransport();

        ~MBedEthernetTransport() override;

        void setSocket(InternetSocket *sock) {
            close();

            socket = sock;
            socket->set_blocking(false);
            lastReadAmt = readPos = writePos = 0;
            isOpen = true;
        }

        void flush() override;

        int writeChar(char data) override;

        int writeStr(const char *data) override;

        uint8_t readByte() override;

        bool readAvailable() override;

        bool available() override;

        bool connected() override {
            return isOpen;
        }

        void close() override;

        void endMsg() override;

        void exec() override {
            if (isOpen && writePos > 0) flush();
        }
    };

    class MbedEthernetInitialisation : public DeviceInitialisation {
    private:
        TCPSocket server;
        NetworkInterface *defNetwork;
        const ConnectorLocalInfo &localInfo;
        int listenPort;
        enum InitialisationState {
            NOT_STARTED, NW_STARTED, SOCKET_OPEN, SOCKET_BOUND, NW_FAILED
        } initState;
    public:
        /**
         * Creates the ethernet client manager components.
         * @param server a ready configured ethernet server instance.
         * @param namePgm the local name in program memory on AVR
         */
        MbedEthernetInitialisation(int listenPort, const ConnectorLocalInfo &localInfo) {
            defNetwork = NetworkInterface::get_default_instance();
        }

        bool attemptInitialisation() override;

        bool attemptNewConnection(TagValueTransport *transport) override;

        /**
         * @return true if the server has now bound to an external address, IE. the network is connected and working
         */
        bool isBound() { return initState == SOCKET_BOUND; }

        /**
         * Gets the network interface that's being used by this connector, only ever use this interface if isBound
         * returns true, because otherwise it could be null or completely undefined.
         * @return the network interface that can be used for other network operations.
         */
        NetworkInterface *networkInterface() { return defNetwork; }

    };

}

#endif //TCMENU_MBEDETHERNETTRANSPORT_H
