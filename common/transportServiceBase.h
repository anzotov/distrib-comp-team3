#pragma once

#include "../common/calcResult.h"
#include "../common/calcTask.h"

#include <QObject>
#include <QList>

#include <stdexcept>

class TransportServiceBase : public QObject
{
    Q_OBJECT
public:
    using PeerHandlerType = QString;
    TransportServiceBase(QObject *parent = nullptr) : QObject(parent) {}
signals:
    void receivedCalcTask(const PeerHandlerType peerHandler, const CalcTask task);
    void receivedCalcResult(const PeerHandlerType peerHandler, const CalcResult result);
    void newPeer(const PeerHandlerType peerHandler);
    void peerDiconnected(const PeerHandlerType peerHandler);
    void connectError(const QString &peerInfo);

public:
    virtual void sendCalcTask(const PeerHandlerType &peerHandler, const CalcTask &task) = 0;
    virtual void sendCalcResult(const PeerHandlerType &peerHandler, const CalcResult &result) = 0;
    virtual void connectPeer(const QString &peerInfo) = 0;
    virtual void disconnectPeer(const PeerHandlerType &peerHandler) = 0;
    virtual QList<PeerHandlerType> peers() const = 0;
};