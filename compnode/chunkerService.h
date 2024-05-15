#pragma once

#include "chunkerServiceBase.h"

#include "peerInfo.h"
#include "calculatorServiceBase.h"
#include <QMap>
#include <chrono>
#include <optional>

class ChunkerService final : public ChunkerServiceBase
{
public:
    ChunkerService(CalculatorServiceBase *calculatorService, QObject *parent = nullptr);
    ~ChunkerService();
    void start() override final;
    void stop() override final;
    void calculateTask(const PeerHandlerType &peerHandler, const CalcTask &task) override final;
    void updatePeers(const QList<PeerInfo> &peers) override final;
    void processChunkedResult(const PeerHandlerType &peerHandler, const CalcResult &result) override final;

private:
    enum class State
    {
        Stopped,
        TestCalcRequested,
        Ready,
        MainTaskReceived,
        ChunkTaskReceived,
    };

    void setStateReady();
    void OnCalculatorServiceCalcDone(CalcResult result);
    void sliceTask(const CalcTask &task);
    void checkChunkedResult();
    void stopCalc();

    State m_state = State::Stopped;
    CalculatorServiceBase *m_calculatorService = nullptr;
    CalcTask m_testTask;
    QString m_compPower;
    std::chrono::steady_clock::time_point m_taskStartTime;
    QMap<PeerHandlerType, PeerInfo> m_peers;
    QMap<PeerHandlerType, std::optional<CalcResult>> m_resultChunks;
    QList<PeerHandlerType> m_resultChunksOrder;
    std::optional<CalcResult> m_result;
    PeerHandlerType m_taskSourceHandler;
};