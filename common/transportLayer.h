#pragma once

#include "transportLayerBase.h"

#include <QTcpSocket>
#include <QTcpServer>
#include <QMap>
#include <QPointer>

class TransportLayer final : public TransportLayerBase
{
public:
    TransportLayer(const quint16 &tcpPortValue,
                   QObject *parent = nullptr);
    ~TransportLayer();

    void sendData(const PeerHandlerType &peerHandler, const QByteArray &data) override final;
    void openConnection(const QString &peerInfo) override final;
    void closeConnection(const PeerHandlerType &peerHandler) override final;
    void startListening() override final;
    void stopListening() override final;
    quint16 getPort() const;
private:
    QMap<PeerHandlerType, QPointer<QTcpSocket>> m_socketMap;
    QTcpServer *m_server;
    quint32 m_maxPeerHandler;
    quint16 m_port;
};
