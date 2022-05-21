/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "BaseRemoteComponents.h"
#include "BaseBufferedRemoteTransport.h"

using namespace tcremote;

void BaseRemoteServerConnection::runLoop() {
    if(!initialisation.isInitialised()) {
        initialisation.attemptInitialisation();
    }
    else if (!connected()) {
        initialisation.attemptNewConnection(this);
    } else {
        tick();
    }
}

TagValueRemoteServerConnection::TagValueRemoteServerConnection(TagValueTransport &transport, DeviceInitialisation &initialisation)
        : BaseRemoteServerConnection(initialisation, TAG_VAL_REMOTE_SERVER), remoteConnector(0),
          remoteTransport(transport), messageProcessor() {
}

void TagValueRemoteServerConnection::tick() {
    remoteConnector.tick();

    // if this is a buffered transport, we must give it chance to flush the buffer from time to time.
    if(remoteTransport.getTransportType() == TVAL_BUFFERED) {
        reinterpret_cast<BaseBufferedRemoteTransport&>(remoteTransport).flushIfRequired();
    }
}

void TagValueRemoteServerConnection::init(int remoteNumber, const ConnectorLocalInfo& info) {
    // first we setup the remote number and initialise the connector
    connector()->initialise(transport(), messageProcessors(), &info, remoteNumber);
}

void TagValueRemoteServerConnection::copyConnectionStatus(char *buffer, int bufferSize) {
    strncpy(buffer, connector()->getRemoteName(), bufferSize);
    buffer[bufferSize - 1] = 0; // make sure it's zero terminated
    appendChar(buffer, ':', bufferSize);
    char authStatus = connector()->isAuthenticated() ? 'A' : (connector()->isConnected() ? 'C' : 'D');
    appendChar(buffer, authStatus, bufferSize);
    appendChar(buffer, ':', bufferSize);
    fastltoa(buffer, connector()->getRemoteMajorVer(), 2, NOT_PADDED, bufferSize);
    appendChar(buffer, '.', bufferSize);
    fastltoa(buffer, connector()->getRemoteMinorVer(), 2, NOT_PADDED, bufferSize);
    if (strlen(buffer) < (unsigned int)bufferSize) {
        appendChar(buffer, ':', bufferSize);
        appendChar(buffer, connector()->getRemotePlatform() + '0', bufferSize);
    }
}

uint8_t tcremote::TcMenuRemoteServer::addConnection(tcremote::BaseRemoteServerConnection *toAdd) {
    if(remotesAdded >= ALLOWED_CONNECTIONS) return 0xff;

    if(remotesAdded == 0) {
        serdebugF("Starting remote server tick handler");
        taskManager.scheduleFixedRate(TICK_INTERVAL, this, TIME_MILLIS);
    }

    serdebugF2("Adding connection #", remotesAdded);

    // and then add it to our array.
    connections[remotesAdded] = toAdd;
    toAdd->init(remotesAdded, appInfo);

    return remotesAdded++;
}

void TcMenuRemoteServer::exec() {
    for (int i = 0; i < remotesAdded; i++) {
        connections[i]->runLoop();
        taskManager.yieldForMicros(0);
    }
}
