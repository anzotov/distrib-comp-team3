#pragma once

#include "transportLayerBase.h"

#include <QTcpSocket>
#include <QTcpServer>
#include <QMap>

class TransportLayer : public TransportLayerBase
{
public:
    TransportLayer();
    ~TransportLayer();

    void sendData(const PeerHandlerType peerHandler, const QByteArray data) override final;
    void openConnection(const QString &peerInfo) override final;
    void closeConnection(const PeerHandlerType peerHandler) override final;
    void startListening() override final;
    void stopListening() override final;
private:
    QMap<QTcpSocket, PeerHandlerType> *m_socket;
};
