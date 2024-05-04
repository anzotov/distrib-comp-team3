#pragma once

#include "../common/calcTask.h"
#include "../common/calcResult.h"
#include "chunkerService.h"
#include "peerService.h"
#include "calculatorService.h"
#include <QObject>

class CompNode final : public QObject
{
    Q_OBJECT
public:
    CompNode(PeerService *peerService,
             ChunkerService *chunkerService,
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

    State m_state = State::Stopped;
    PeerService *m_peerService;
    ChunkerService *m_chunkerService;
};
