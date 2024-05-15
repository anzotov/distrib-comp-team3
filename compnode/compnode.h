#pragma once

#include "../common/calcTask.h"
#include "../common/calcResult.h"
#include "chunkerServiceBase.h"
#include "peerServiceBase.h"
#include "calculatorServiceBase.h"
#include <QObject>

class CompNode final : public QObject
{
    Q_OBJECT
public:
    CompNode(PeerServiceBase *peerService,
             ChunkerServiceBase *chunkerService,
             QObject *parent = nullptr);
    ~CompNode();
    void start();
    void stop();
signals:
    void stopped();

private:
    enum class State
    {
        Stopped,
        Starting,
        Ready,
        TaskReceived,
    };

    void onChunkerServiceReady(const QString compPower);
    void onChunkerServiceCalcResult(const PeerHandlerType peerHandler, const CalcResult result);
    void onChunkerServiceCalcError();
    void onChunkerServiceSendChunkedTask(const PeerHandlerType peerHandler, const CalcTask task);
    void onReceivedCalcTask(const PeerHandlerType peerHandler, const CalcTask task);
    void onReceivedCalcResult(const PeerHandlerType peerHandler, const CalcResult result);
    void onPeersChanged(const QList<PeerInfo> peers);
    void onTaskNodeDisconnected(const PeerHandlerType peerHandler);
    void restartNode();

    State m_state = State::Stopped;
    PeerServiceBase *m_peerService = nullptr;
    ChunkerServiceBase *m_chunkerService = nullptr;
    PeerHandlerType m_taskSourceHandler;
};
