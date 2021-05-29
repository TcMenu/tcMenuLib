/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "BaseRemoteComponents.h"

using namespace tcremote;

tcremote::RemoteServerConnection::RemoteServerConnection(TagValueTransport &transport, DeviceInitialisation &initialisation)
        : remoteConnector(0), remoteTransport(transport), messageProcessor(msgHandlers, MSG_HANDLERS_SIZE),
          initialisation(initialisation) {
}

void RemoteServerConnection::tick() {
    if (initialisation.isInitialised()) {
        remoteConnector.tick();
        if (!remoteTransport.connected()) {
            initialisation.attemptNewConnection(transport());
        }
    }
    else {
        initialisation.attemptInitialisation();
    }
}

uint8_t tcremote::TcMenuRemoteServer::addConnection(tcremote::RemoteServerConnection *toAdd) {
    if(remotesAdded >= ALLOWED_CONNECTIONS) return 0xff;

    if(remotesAdded == 0) {
        serdebugF("Starting remote server tick handler");
        taskManager.scheduleFixedRate(TICK_INTERVAL, this, TIME_MILLIS);
    }

    serdebugF2("Adding connection #", remotesAdded);

    // first we setup the remote number and initialise the connector
    int remoteNo = remotesAdded;
    toAdd->connector()->initialise(toAdd->transport(), toAdd->messageProcessors(), &appInfo, remoteNo);

    // if there is an authenticator present, we add it to the connection
    if (menuMgr.getAuthenticator()) {
        toAdd->connector()->setAuthManager(menuMgr.getAuthenticator());
    }

    // and then add it to our array.
    connections[remotesAdded++] = toAdd;

    return remoteNo;
}

void TcMenuRemoteServer::exec() {
    for (int i = 0; i < remotesAdded; i++) {
        connections[i]->tick();
        taskManager.yieldForMicros(0);
    }
}
