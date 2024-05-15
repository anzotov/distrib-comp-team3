#pragma once

#include <QObject>
#include <QByteArray>

#include <stdexcept>

class TransportLayerBase : public QObject
{
    Q_OBJECT

public:
    using PeerHandlerType = QString;
    TransportLayerBase(QObject *parent = nullptr) : QObject(parent) {}
    
signals:
    void newConnection(const PeerHandlerType peerHandler, const QString peerInfo, bool outgoing);
    void connectionClosed(const PeerHandlerType peerHandler);
    void connectError(const QString peerInfo);
    void dataReceived(const PeerHandlerType peerHandler, const QByteArray data);

public:
    virtual void sendData(const PeerHandlerType& peerHandler, const QByteArray &data) = 0;
    virtual void openConnection(const QString &peerInfo) = 0;
    virtual void closeConnection(const PeerHandlerType& peerHandler) = 0;
    virtual void startListening() { throw std::logic_error("Not implemented"); }
    virtual void stopListening() { throw std::logic_error("Not implemented"); }
};