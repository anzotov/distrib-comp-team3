#pragma once

#include "../compnode/chunkerServiceBase.h"

#include <functional>

struct ChunkerServiceMock final : ChunkerServiceBase
{
    ChunkerServiceMock(
        std::function<void(ChunkerServiceMock *)> constructor, QObject *parent = nullptr)
        : ChunkerServiceBase(parent)
    {
        constructor(this);
    }

    ~ChunkerServiceMock()
    {
        if (m_destructor)
            m_destructor(this);
    }

    void start() override final
    {
        ++m_startCount;
        if (m_start)
            m_start(this);
    }
    void stop() override final
    {
        ++m_stopCount;
        if (m_stop)
            m_stop(this);
    }
    void calculateTask(const PeerHandlerType &peerHandler, const CalcTask &task) override final
    {
        ++m_calculateTaskCount;
        if (m_calculateTask)
            m_calculateTask(this, peerHandler, task);
    }
    void updatePeers(const QList<PeerInfo> &peers) override final
    {
        ++m_updatePeersCount;
        if (m_updatePeers)
            m_updatePeers(this, peers);
    }
    void processChunkedResult(const PeerHandlerType &peerHandler, const CalcResult &result) override final
    {
        ++m_processChunkedResultCount;
        if (m_processChunkedResult)
            m_processChunkedResult(this, peerHandler, result);
    }

    std::function<void(ChunkerServiceMock *)> m_start;
    mutable long m_startCount = 0;
    std::function<void(ChunkerServiceMock *)> m_stop;
    mutable long m_stopCount = 0;
    std::function<void(ChunkerServiceMock *, const PeerHandlerType &, const CalcTask &)> m_calculateTask;
    mutable long m_calculateTaskCount = 0;
    std::function<void(ChunkerServiceMock *, const QList<PeerInfo> &)> m_updatePeers;
    mutable long m_updatePeersCount = 0;
    std::function<void(ChunkerServiceMock *, const PeerHandlerType &, const CalcResult &)> m_processChunkedResult;
    mutable long m_processChunkedResultCount = 0;
    std::function<void(ChunkerServiceMock *)> m_destructor;
};
