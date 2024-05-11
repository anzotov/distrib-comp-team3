#pragma once

#include "../common/handshake.h"
#include "../common/calcResult.h"
#include "../common/calcTask.h"
#include "transportLayerBase.h"

#include <QObject>
#include <QList>

#include <stdexcept>

class TransportServiceBase : public QObject
{
    Q_OBJECT
public:
    using PeerHandlerType = TransportLayerBase::PeerHandlerType;
    TransportServiceBase(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~TransportServiceBase() = default;
signals:
    void receivedHandshake(const PeerHandlerType peerHandler, const Handshake handshake);
    void receivedCalcTask(const PeerHandlerType peerHandler, const CalcTask task);
    void receivedCalcResult(const PeerHandlerType peerHandler, const CalcResult result);
    void newPeer(const PeerHandlerType peerHandler, const QString peerInfo, bool outgoing);
    void peerDiconnected(const PeerHandlerType peerHandler);
    void connectError(const QString &peerInfo);

public:
    virtual void sendHandshake(const PeerHandlerType &peerHandler, const Handshake &handshake) = 0;
    virtual void sendCalcTask(const PeerHandlerType &peerHandler, const CalcTask &task) = 0;
    virtual void sendCalcResult(const PeerHandlerType &peerHandler, const CalcResult &result) = 0;
    virtual void connectPeer(const QString &peerInfo) = 0;
    virtual void disconnectPeer(const PeerHandlerType &peerHandler) = 0;
    virtual void disconnectAllPeers() = 0;
    virtual QList<PeerHandlerType> peers() const = 0;
    virtual void startListening() { throw std::logic_error("Not implemented"); }
    virtual void stopListening() { throw std::logic_error("Not implemented"); }
};