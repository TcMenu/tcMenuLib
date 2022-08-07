
#include <Arduino.h>
#include <remote/TransportNetworkDriver.h>
#include <TaskManagerIO.h>
#include "utility/stm32_eth.h"
#include "utility/ethernetif.h"
#include "tcUtil.h"
#include "lwip/timeouts.h"
#include "IoLogging.h"

#define MAX_TCP_ACCEPTS 2
#define MAX_TCP_CLIENTS 6
#define WRITE_BUFFER_SIZE 128

extern struct netif gnetif;

namespace tcremote {
    err_t tcpDataWasReceived(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
    void tcpErrorCallback(void *arg, err_t err);
    err_t tcpDataSentCallback(void *arg, struct tcp_pcb *tpcb, u16_t len);

    const uint8_t* myMacAddress;

    class StmTcpClient {
    private:
        tcp_struct clientStruct;
        uint8_t writeBuffer[WRITE_BUFFER_SIZE];
        uint16_t writeBufferPos;
        uint16_t bytesWeAreWaitingFor;
        uint16_t timeOutMillis;
        uint16_t lastWriteTick;
    public:
        StmTcpClient() : clientStruct{}, writeBuffer{}, writeBufferPos(0), bytesWeAreWaitingFor(0), timeOutMillis(1000), lastWriteTick(0) {}

        void initialise(tcp_pcb* pcb) {
            clientStruct.pcb = pcb;
            clientStruct.state = TCP_ACCEPTED;
            clientStruct.data.p = nullptr;
            clientStruct.data.available = 0;
            writeBufferPos = 0;
            bytesWeAreWaitingFor = 0;
            lastWriteTick = 0;

            tcp_arg(pcb, this);
            tcp_recv(pcb, tcpDataWasReceived);
            tcp_err(pcb, tcpErrorCallback);
            tcp_sent(pcb, tcpDataSentCallback);
        }

        bool isInUse() const { return clientStruct.pcb != nullptr; }

        SocketErrCode flush() {
            if ((clientStruct.state != TCP_ACCEPTED) && (clientStruct.state != TCP_CONNECTED)) {
                return SOCK_ERR_FAILED;
            }

            if(writeBufferPos == 0) return SOCK_ERR_OK; // nothing to do
            auto err = doRawTcpWrite(writeBuffer, writeBufferPos);
            writeBufferPos = 0;

            return err;
        }

        SocketErrCode doRawTcpWrite(const uint8_t* buffer, size_t len) {
            size_t left = len;
            uint32_t then = millis();

            do {
                size_t maxSendSize = tcp_sndbuf(clientStruct.pcb);

                if(bytesWeAreWaitingFor > 5000 || maxSendSize < 50) {
                    // if we are getting ahead of ourselves by too far, or we've run out of space we back off
                    taskManager.yieldForMicros(50);
                } else {
                    size_t thisTime = left > maxSendSize ? maxSendSize : left;
                    auto err = tcp_write(clientStruct.pcb, &writeBuffer, writeBufferPos, TCP_WRITE_FLAG_COPY);
                    if (err != ERR_OK) {
                        serdebugF("Socket write error");
                        return SOCK_ERR_FAILED;
                    }

                    bytesWeAreWaitingFor += left;
                    left -= thisTime;

                    if (ERR_OK != tcp_output(clientStruct.pcb)) {
                        return SOCK_ERR_FAILED;
                    }

                }

                stm32_eth_scheduler();

                if ((millis() - then) > timeOutMillis) return SOCK_ERR_TIMEOUT;
            } while(left > 0);
            return SOCK_ERR_OK;
        }

        void tick() {
            if(clientStruct.pcb != nullptr && lastWriteTick > 0) {
                --lastWriteTick;
                if(lastWriteTick == 0 && writeBufferPos > 0) flush();
            }
        }

        void close() {
            if(clientStruct.pcb) {
                tcp_connection_close(clientStruct.pcb, &clientStruct);
            }
        }

        int read(uint8_t * buffer, size_t bufferSize) {
            if(clientStruct.data.p != nullptr) {
                return (int) stm32_get_data(&clientStruct.data, buffer, bufferSize);
            }
            // No data available
            return 0;
        }

        SocketErrCode write(uint8_t data) {
            if(writeBufferPos >= WRITE_BUFFER_SIZE) {
                auto ret = flush();
                if(ret != SOCK_ERR_OK) {
                    close();
                    return ret;
                }
            }

            writeBuffer[writeBufferPos++] = data;
            if(lastWriteTick == 0) lastWriteTick = 3;
            return SOCK_ERR_OK;
        }

        SocketErrCode write(const void* data, size_t len, int timeout) {
            timeOutMillis = timeout;
            if(len > WRITE_BUFFER_SIZE) {
                // this is a large data set, flush what we've got and send in one go
                if(flush() != SOCK_ERR_OK) return SOCK_ERR_FAILED;
                return doRawTcpWrite((uint8_t*)data, len);
            } else {
                for (size_t i = 0; i < len; i++) {
                    auto ret = write(((uint8_t *) data)[i]);
                    if (ret != SOCK_ERR_OK) return ret;
                }
            }
            return SOCK_ERR_OK;
        }

        void errorOccurred(err_t e) {
            if(e != ERR_OK) {
                serdebugF2("Socket err ", e)
                close();
            }
        }

        err_t dataRx(tcp_pcb* pcb, pbuf* p, err_t err) {
            err_t ret_err;

            /* if we receive an empty tcp frame from server => close connection */
            if (p == nullptr) {
                /* we're done sending, close connection */
                close();
                ret_err = ERR_OK;
            } else if (err != ERR_OK) {
                /* free received pbuf*/
                pbuf_free(p);
                ret_err = err;
            } else if ((clientStruct.state == TCP_CONNECTED) || (clientStruct.state == TCP_ACCEPTED)) {
                /* Acknowledge data reception and store*/
                tcp_recved(pcb, p->tot_len);

                if (clientStruct.data.p == nullptr) {
                    clientStruct.data.p = p;
                } else {
                    pbuf_chain(clientStruct.data.p, p);
                }

                clientStruct.data.available += p->len;
                ret_err = ERR_OK;
            } else {
                /* data received when connection already closed */
                /* Acknowledge data reception */
                tcp_recved(pcb, p->tot_len);

                /* free pbuf and do nothing */
                pbuf_free(p);
                ret_err = ERR_OK;
            }
            return ret_err;
        }

        void dataWasSent(tcp_pcb* /*pcb*/, uint16_t amount) {
            bytesWeAreWaitingFor -= amount;
        }

        bool readAvailable() {
            return clientStruct.data.available;
        }

        bool writeAvailable() {
            return bytesWeAreWaitingFor < 500;
        }
    };

    err_t tcpDataWasReceived(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
        auto* client = reinterpret_cast<StmTcpClient*>(arg);
        return client->dataRx(tpcb, p, err);
    }

    void tcpErrorCallback(void *arg, err_t err) {
        auto* client = reinterpret_cast<StmTcpClient*>(arg);
        client->errorOccurred(err);
    }

    err_t tcpDataSentCallback(void *arg, struct tcp_pcb *tpcb, u16_t len) {
        auto* client = reinterpret_cast<StmTcpClient*>(arg);
        client->dataWasSent(tpcb, len);
        return ERR_OK;
    }

    StmTcpClient tcpClients[MAX_TCP_CLIENTS];

    class StmTcpServer : public BaseEvent {
    private:
        uint16_t portNum;
        tcp_struct tcpServer;
        void* userData;
        ServerAcceptedCallback theCallback;
        tcp_pcb volatile* queuedPcb;
    public:
        StmTcpServer() : portNum(0), tcpServer{}, userData(nullptr), theCallback(nullptr), queuedPcb(nullptr) {
        }

        uint16_t getPortNum() const { return portNum; }

        bool initialise(uint16_t port, ServerAcceptedCallback cb, void *theData);
        void onNewClient(tcp_pcb* clientPcb);

        void exec() override;

        uint32_t timeOfNextCheck() override;
    };

    err_t tcpConnectionEstablished(void *arg, struct tcp_pcb *newpcb, err_t err) {
        if(ERR_OK != err) {
            serdebugF("Accept error");
            return err;
        } else {
            auto tcpServer = reinterpret_cast<StmTcpServer *>(arg);
            tcpServer->onNewClient(newpcb);
            return ERR_OK;
        }
    }

    bool StmTcpServer::initialise(uint16_t port, ServerAcceptedCallback cb, void *theData) {
        taskManager.registerEvent(this);

        tcpServer.pcb = tcp_new();
        tcp_arg(tcpServer.pcb, this);
        tcpServer.state = TCP_NONE;

        if (ERR_OK != tcp_bind(tcpServer.pcb, IP_ADDR_ANY, port)) {
            memp_free(MEMP_TCP_PCB, tcpServer.pcb);
            tcpServer.pcb = nullptr;
            return false;
        }

        tcpServer.pcb = tcp_listen(tcpServer.pcb);
        theCallback = cb;
        userData = theData;
        tcp_accept(tcpServer.pcb, tcpConnectionEstablished);
        portNum = port;
        return true;
    }

    socket_t nextFreeClient() {
        for(int i=0; i<MAX_TCP_CLIENTS; i++) {
            if(!tcpClients[i].isInUse()) return i;
        }
        return TC_BAD_SOCKET_ID;
    }

    void StmTcpServer::onNewClient(tcp_pcb* clientPcb) {
        tcp_setprio(clientPcb, TCP_PRIO_MIN);
        if(queuedPcb == nullptr) {
            queuedPcb = clientPcb;
            markTriggeredAndNotify();
        }
    }

    void StmTcpServer::exec() {
        if(queuedPcb != nullptr) {
            auto pcb = (tcp_pcb*)queuedPcb;
            queuedPcb = nullptr;
            int client = nextFreeClient();
            if(client == TC_BAD_SOCKET_ID) return;
            tcpClients[client].initialise(pcb);
            theCallback(client, userData);
        }
    }

    uint32_t StmTcpServer::timeOfNextCheck() {
        return secondsToMicros(1);
    }

    StmTcpServer tcpSlots[MAX_TCP_ACCEPTS];

    SocketErrCode startNetLayerDhcp() {
        return SOCK_ERR_UNSUPPORTED;
    }

    SocketErrCode startNetLayerManual(const uint8_t* ip, const uint8_t* mac, const uint8_t* mask) {
        stm32_eth_init(mac, ip, ip, mask);
        /* If there is a local DHCP informs it of our manual IP configuration to
        prevent IP conflict */
        stm32_DHCP_manual_config();
        myMacAddress = mac;

        taskManager.scheduleFixedRate(100, [] {
            for(auto & tcpClient : tcpClients) {
                tcpClient.tick();
            }
        });

        return SOCK_ERR_OK;
    }

    void copyIpIntoBuffer(uint32_t ip, char* buffer, int bufferSize) {
        ltoaClrBuff(buffer, int(ip & 0xff), 3, NOT_PADDED, bufferSize);
        appendChar(buffer, '.', bufferSize);
        fastltoa(buffer, int(ip >> 8) & 0xff, 3, NOT_PADDED, bufferSize);
        appendChar(buffer, '.', bufferSize);
        fastltoa(buffer, int(ip >> 16) & 0xff, 3, NOT_PADDED, bufferSize);
        appendChar(buffer, '.', bufferSize);
        fastltoa(buffer, int(ip >> 24) & 0xff, 3, NOT_PADDED, bufferSize);
    }

    void copyIpAddress(socket_t theSocket, char* buffer, size_t bufferSize) {
        if(theSocket == TC_LOCALHOST_SOCKET_ID) {
            copyIpIntoBuffer(stm32_eth_get_ipaddr(), buffer, int(bufferSize));
        } else {
            buffer[0]=0;
        }
    }

    bool isNetworkUp() {
        return !(!stm32_eth_is_init()) && stm32_eth_link_up() != 0;
    }

    SocketErrCode initialiseAccept(int port, ServerAcceptedCallback onServerAccepted, void* callbackData) {
        for(int i=0; i<MAX_TCP_ACCEPTS; i++) {
            if(tcpSlots[i].getPortNum() == 0) {
                tcpSlots[i].initialise(port, onServerAccepted, callbackData);
                return SOCK_ERR_OK;
            }
        }
        return SOCK_ERR_FAILED;

    }

    int rawReadData(socket_t socketNum, void* data, size_t dataLen) {
        if(socketNum < 0 || socketNum >= MAX_TCP_CLIENTS || !tcpClients[socketNum].isInUse()) return -1;
        return tcpClients[socketNum].read((uint8_t*)data, dataLen);
    }

    bool rawWriteAvailable(socket_t socketNum) {
        if(socketNum < 0 || socketNum >= MAX_TCP_CLIENTS || !tcpClients[socketNum].isInUse()) return false;
        return tcpClients[socketNum].writeAvailable();
    }


    bool rawReadAvailable(socket_t socketNum) {
        if(socketNum < 0 || socketNum >= MAX_TCP_CLIENTS || !tcpClients[socketNum].isInUse()) return false;
        return tcpClients[socketNum].readAvailable();
    }

    SocketErrCode rawWriteData(socket_t socketNum, const void* data, size_t dataLen, int timeoutMillis) {
        if(socketNum < 0 || socketNum >= MAX_TCP_CLIENTS || !tcpClients[socketNum].isInUse()) return SOCK_ERR_FAILED;
        return tcpClients[socketNum].write(data, dataLen, timeoutMillis);
    }

    SocketErrCode rawFlushAll(socket_t socketNum) {
        if(socketNum < 0 || socketNum >= MAX_TCP_CLIENTS || !tcpClients[socketNum].isInUse()) return SOCK_ERR_FAILED;
        return tcpClients[socketNum].flush();
    }

    void closeSocket(socket_t socketNum) {
        if(socketNum < 0 || socketNum >= MAX_TCP_CLIENTS || !tcpClients[socketNum].isInUse()) return;
        tcpClients[socketNum].close();
    }
}
