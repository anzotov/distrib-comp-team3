#pragma once

#include "../common/calcTask.h"
#include "../common/calcResult.h"
#include "peerInfo.h"

#include <QObject>

class ChunkerService : public QObject
{
    Q_OBJECT
public:
    ChunkerService(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ChunkerService() = default;

signals:
    void ready(const QString compPower);
    void calcResult(const PeerHandlerType peerHandler, const CalcResult result);
    void calcError();
    void sendChunkedTask(const PeerHandlerType peerHandler, const CalcTask task);

public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void calculateTask(const PeerHandlerType &peerHandler, const CalcTask &task) = 0;
    virtual void updatePeers(const QList<PeerInfo> &peers) = 0;
    virtual void processChunkedResult(const PeerHandlerType &peerHandler, const CalcResult &result) = 0;
};