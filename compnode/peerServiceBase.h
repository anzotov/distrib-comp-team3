#pragma once

#include "../common/transportServiceBase.h"
#include "peerInfo.h"

#include <QObject>
#include <QList>

class PeerServiceBase : public QObject
{
    Q_OBJECT
public:
    PeerServiceBase(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~PeerServiceBase() = default;

signals:
    void receivedCalcTask(const PeerHandlerType peerHandler, const CalcTask task);
    void receivedCalcResult(const PeerHandlerType peerHandler, const CalcResult result);
    void peersChanged(QList<PeerInfo> peers);
    void taskNodeDisconnected(const PeerHandlerType peerHandler);

public:
    virtual void start(const QString &compPower) = 0;
    virtual void stop() = 0;
    virtual void sendCalcTask(const PeerHandlerType &peerHandler, const CalcTask &task) = 0;
    virtual void sendCalcResult(const PeerHandlerType &peerHandler, const CalcResult &result) = 0;
    virtual QList<PeerInfo> peers() const = 0;
};