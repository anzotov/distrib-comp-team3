#pragma once

#include "taskProvider.h"

#include "../common/calcTask.h"
#include "../common/calcResult.h"
#include "../common/transportServiceBase.h"

#include <QObject>
#include <QString>
#include <optional>

class TaskNode final : public QObject
{
    Q_OBJECT

public:
    TaskNode(TransportServiceBase *transportServiceBase,
             TaskProvider *taskProvider,
             const QString &peerInfo,
             int connectRetries = 10,
             int reconnectTime = 1000,
             QObject *parent = nullptr);
    ~TaskNode();
    void start();
    void stop();

signals:
    void stopped(bool success);

private:
    enum class State
    {
        Stopped,
        TaskRequested,
        ConnectionRequested,
        HandshakeSent,
        TaskSent,
        FormatRequested,
    };

    void onReceivedHandshake(const TransportServiceBase::PeerHandlerType peerHandler, const Handshake handshake);
    void onReceivedCalcResult(const TransportServiceBase::PeerHandlerType peerHandler,
                              const CalcResult result);
    void onNewPeer(const TransportServiceBase::PeerHandlerType peerHandler, const QString peerInfo, bool outgoing);
    void onConnectError(const QString &peerInfo);
    void onPeerDiconnected(const TransportServiceBase::PeerHandlerType peerHandler);
    void onTaskLoadDone(const CalcTask task);
    void onTaskLoadError();
    void onNoTasksAvailable();
    void onResultFormatDone();
    void onResultFormatError(CalcResult result);
    void sendHandshake(const TransportServiceBase::PeerHandlerType &peerHandler);

    TransportServiceBase *m_transportServiceBase = nullptr;
    TaskProvider *m_taskProvider = nullptr;

    State m_state = State::Stopped;
    const QString m_peerInfo;
    const int m_maxConnectAttempts = 1;
    const int m_reconnectTime = 1000;
    int m_connectAttempt = 0;
    std::optional<CalcTask> m_task;
    std::optional<TransportServiceBase::PeerHandlerType> m_peerHandler;
};