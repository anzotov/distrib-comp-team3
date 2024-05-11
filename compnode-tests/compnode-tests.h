#pragma once

#include "../compnode/compnode.h"

#include <QObject>
#include <QTest>
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

struct PeerServiceMock final : PeerServiceBase
{
    PeerServiceMock(
        std::function<void(PeerServiceMock *)> constructor, QObject *parent = nullptr)
        : PeerServiceBase(parent)
    {
        constructor(this);
    }

    ~PeerServiceMock()
    {
        if (m_destructor)
            m_destructor(this);
    }

    void start(const QString &compPower) override final
    {
        ++m_startCount;
        m_start(this, compPower);
    }
    void stop() override final
    {
        ++m_stopCount;
        m_stop(this);
    }
    void sendCalcTask(const PeerHandlerType &peerHandler, const CalcTask &task) override final
    {
        ++m_sendCalcTaskCount;
        m_sendCalcTask(this, peerHandler, task);
    }
    void sendCalcResult(const PeerHandlerType &peerHandler, const CalcResult &result) override final
    {
        ++m_sendCalcResultCount;
        m_sendCalcResult(this, peerHandler, result);
    }
    QList<PeerInfo> peers() const override final
    {
        ++m_peersCount;
        return m_peers(this);
    }
    std::function<void(PeerServiceMock *, const QString &)> m_start;
    mutable long m_startCount = 0;
    std::function<void(PeerServiceMock *)> m_stop;
    mutable long m_stopCount = 0;
    std::function<void(PeerServiceMock *, const PeerHandlerType &, const CalcTask &)> m_sendCalcTask;
    mutable long m_sendCalcTaskCount = 0;
    std::function<void(PeerServiceMock *, const PeerHandlerType &, const CalcResult &)> m_sendCalcResult;
    mutable long m_sendCalcResultCount = 0;
    std::function<QList<PeerInfo>(const PeerServiceMock *)> m_peers;
    mutable long m_peersCount = 0;
    std::function<void(PeerServiceMock *)> m_destructor;
};

class CompNodeTestCase : public QObject
{
    Q_OBJECT

private slots:
    void PositiveTest()
    {
        CalcTask test_task("sin(x)", {"0", "1"}, true);
        CalcTask test_chunkedTask("sin(x)", {"1"}, false);
        CalcResult test_result({"0", "1"}, true);
        CalcResult test_chunkedResult({"1"}, false);
        int test_chunkedResultsReceived = 0;
        PeerHandlerType test_taskNodeHandler = "123";
        QList<PeerInfo> test_peers = {
            {"111", "1000", "10"},
            {"112", "100", "100"},
        };
        QString test_compPower = "1000";
        auto test_peerServiceMock = new PeerServiceMock([&](PeerServiceMock *) {});
        test_peerServiceMock->m_start =
            [&](PeerServiceMock *self, const QString &compPower)
        {
            QCOMPARE(compPower, test_compPower);
            emit self->peersChanged(test_peers);
        };
        test_peerServiceMock->m_stop =
            [&](PeerServiceMock *) {};
        test_peerServiceMock->m_sendCalcTask =
            [&](PeerServiceMock *self, const PeerHandlerType &peerHandler, const CalcTask &task)
        {
            QCOMPARE(task, test_chunkedTask);
            emit self->receivedCalcResult(peerHandler, test_chunkedResult);
        };
        test_peerServiceMock->m_sendCalcResult =
            [&](PeerServiceMock *, const PeerHandlerType &peerHandler, const CalcResult &result)
        {
            QCOMPARE(peerHandler, test_taskNodeHandler);
            QCOMPARE(result, test_result);
        };
        test_peerServiceMock->m_peers =
            [&](const PeerServiceMock *)
        { return test_peers; };

        auto test_chunkerServiceMock = new ChunkerServiceMock([&](ChunkerServiceMock *) {});
        test_chunkerServiceMock->m_start =
            [&](ChunkerServiceMock *self)
        {
            emit self->ready(test_compPower);
        };
        test_chunkerServiceMock->m_stop =
            [&](ChunkerServiceMock *) {};
        test_chunkerServiceMock->m_calculateTask =
            [&](ChunkerServiceMock *self, const PeerHandlerType &peerHandler, const CalcTask &task)
        {
            QCOMPARE(peerHandler, test_taskNodeHandler);
            QCOMPARE(task, test_task);
            for (const auto &peer : test_peers)
            {
                emit self->sendChunkedTask(peer.peerHandler, test_chunkedTask);
            }
        };
        test_chunkerServiceMock->m_updatePeers =
            [&](ChunkerServiceMock *, const QList<PeerInfo> &peers)
        {
            QCOMPARE(peers.size(), test_peers.size());
            for (int i = 0; i < peers.size(); ++i)
            {
                QCOMPARE(peers.at(i), test_peers.at(i));
            }
        };
        test_chunkerServiceMock->m_processChunkedResult =
            [&](ChunkerServiceMock *self, const PeerHandlerType &peerHandler, const CalcResult &result)
        {
            ++test_chunkedResultsReceived;
            QCOMPARE(result, test_chunkedResult);

            bool found_peer = false;
            for (const auto &peer : test_peers)
            {
                if (peer.peerHandler == peerHandler)
                {
                    found_peer = true;
                    break;
                }
            }
            QVERIFY(found_peer);

            if (test_chunkedResultsReceived == test_peers.size())
            {
                emit self->calcResult(test_taskNodeHandler, test_result);
            }
        };
        CompNode node(test_peerServiceMock, test_chunkerServiceMock);
        node.start();
        QTRY_COMPARE(test_chunkerServiceMock->m_startCount, 1);
        QTRY_COMPARE(test_peerServiceMock->m_startCount, 1);
        QTRY_COMPARE(test_chunkerServiceMock->m_updatePeersCount, 1);

        emit test_peerServiceMock->receivedCalcTask(test_taskNodeHandler, test_task);
        QTRY_COMPARE(test_chunkerServiceMock->m_calculateTaskCount, 1);
        QTRY_COMPARE(test_peerServiceMock->m_sendCalcTaskCount, 2);
        QTRY_COMPARE(test_chunkerServiceMock->m_processChunkedResultCount, 2);
        QTRY_COMPARE(test_peerServiceMock->m_sendCalcResultCount, 1);

        node.stop();
        QTRY_COMPARE(test_chunkerServiceMock->m_stopCount, 1);
        QTRY_COMPARE(test_peerServiceMock->m_stopCount, 1);
    }
    void CalcErrorTest()
    {
        CalcTask test_task("sin(x)", {"0", "1"}, true);
        PeerHandlerType test_taskNodeHandler = "123";
        QString test_compPower = "1000";
        auto test_peerServiceMock = new PeerServiceMock([&](PeerServiceMock *) {});
        test_peerServiceMock->m_start =
            [&](PeerServiceMock *self, const QString &compPower)
        {
            QCOMPARE(compPower, test_compPower);
            emit self->peersChanged({});
        };
        test_peerServiceMock->m_stop =
            [&](PeerServiceMock *) {};
        test_peerServiceMock->m_sendCalcTask =
            [&](PeerServiceMock *, const PeerHandlerType &, const CalcTask &)
        {
            QFAIL("Shouldn't be called in this test");
        };
        test_peerServiceMock->m_sendCalcResult =
            [&](PeerServiceMock *, const PeerHandlerType &, const CalcResult &)
        {
            QFAIL("Shouldn't be called in this test");
        };
        test_peerServiceMock->m_peers =
            [&](const PeerServiceMock *)
        { return QList<PeerInfo>{}; };

        auto test_chunkerServiceMock = new ChunkerServiceMock([&](ChunkerServiceMock *) {});
        test_chunkerServiceMock->m_start =
            [&](ChunkerServiceMock *self)
        {
            emit self->ready(test_compPower);
        };
        test_chunkerServiceMock->m_stop =
            [&](ChunkerServiceMock *) {};
        test_chunkerServiceMock->m_calculateTask =
            [&](ChunkerServiceMock *self, const PeerHandlerType &peerHandler, const CalcTask &task)
        {
            QCOMPARE(peerHandler, test_taskNodeHandler);
            QCOMPARE(task, test_task);
            emit self->calcError();
        };
        test_chunkerServiceMock->m_updatePeers =
            [&](ChunkerServiceMock *, const QList<PeerInfo> &peers)
        {
            QVERIFY(peers.empty());
        };
        test_chunkerServiceMock->m_processChunkedResult =
            [&](ChunkerServiceMock *, const PeerHandlerType &, const CalcResult &)
        {
            QFAIL("Shouldn't be called in this test");
        };
        CompNode node(test_peerServiceMock, test_chunkerServiceMock);
        node.start();
        QTRY_COMPARE(test_chunkerServiceMock->m_startCount, 1);
        QTRY_COMPARE(test_peerServiceMock->m_startCount, 1);
        QTRY_COMPARE(test_chunkerServiceMock->m_updatePeersCount, 1);

        emit test_peerServiceMock->receivedCalcTask(test_taskNodeHandler, test_task);
        QTRY_COMPARE(test_chunkerServiceMock->m_calculateTaskCount, 1);
        QTRY_COMPARE(test_chunkerServiceMock->m_stopCount, 1);
        QTRY_COMPARE(test_peerServiceMock->m_stopCount, 1);
        QTRY_COMPARE(test_chunkerServiceMock->m_startCount, 2);
        QTRY_COMPARE(test_peerServiceMock->m_startCount, 2);
        QTRY_COMPARE(test_chunkerServiceMock->m_updatePeersCount, 2);
    }
    void TaskNodeDisconnectTest()
    {
        CalcTask test_task("sin(x)", {"0", "1"}, true);
        PeerHandlerType test_taskNodeHandler = "123";
        QString test_compPower = "1000";
        auto test_peerServiceMock = new PeerServiceMock([&](PeerServiceMock *) {});
        test_peerServiceMock->m_start =
            [&](PeerServiceMock *self, const QString &compPower)
        {
            QCOMPARE(compPower, test_compPower);
            emit self->peersChanged({});
        };
        test_peerServiceMock->m_stop =
            [&](PeerServiceMock *) {};
        test_peerServiceMock->m_sendCalcTask =
            [&](PeerServiceMock *, const PeerHandlerType &, const CalcTask &)
        {
            QFAIL("Shouldn't be called in this test");
        };
        test_peerServiceMock->m_sendCalcResult =
            [&](PeerServiceMock *, const PeerHandlerType &, const CalcResult &)
        {
            QFAIL("Shouldn't be called in this test");
        };
        test_peerServiceMock->m_peers =
            [&](const PeerServiceMock *)
        { return QList<PeerInfo>{}; };

        auto test_chunkerServiceMock = new ChunkerServiceMock([&](ChunkerServiceMock *) {});
        test_chunkerServiceMock->m_start =
            [&](ChunkerServiceMock *self)
        {
            emit self->ready(test_compPower);
        };
        test_chunkerServiceMock->m_stop =
            [&](ChunkerServiceMock *) {};
        test_chunkerServiceMock->m_calculateTask =
            [&](ChunkerServiceMock *, const PeerHandlerType &peerHandler, const CalcTask &task)
        {
            QCOMPARE(peerHandler, test_taskNodeHandler);
            QCOMPARE(task, test_task);
            emit test_peerServiceMock->taskNodeDisconnected(peerHandler);
        };
        test_chunkerServiceMock->m_updatePeers =
            [&](ChunkerServiceMock *, const QList<PeerInfo> &peers)
        {
            QVERIFY(peers.empty());
        };
        test_chunkerServiceMock->m_processChunkedResult =
            [&](ChunkerServiceMock *, const PeerHandlerType &, const CalcResult &)
        {
            QFAIL("Shouldn't be called in this test");
        };
        CompNode node(test_peerServiceMock, test_chunkerServiceMock);
        node.start();
        QTRY_COMPARE(test_chunkerServiceMock->m_startCount, 1);
        QTRY_COMPARE(test_peerServiceMock->m_startCount, 1);
        QTRY_COMPARE(test_chunkerServiceMock->m_updatePeersCount, 1);

        emit test_peerServiceMock->receivedCalcTask(test_taskNodeHandler, test_task);
        QTRY_COMPARE(test_chunkerServiceMock->m_calculateTaskCount, 1);
        QTRY_COMPARE(test_chunkerServiceMock->m_stopCount, 1);
        QTRY_COMPARE(test_peerServiceMock->m_stopCount, 1);
        QTRY_COMPARE(test_chunkerServiceMock->m_startCount, 2);
        QTRY_COMPARE(test_peerServiceMock->m_startCount, 2);
        QTRY_COMPARE(test_chunkerServiceMock->m_updatePeersCount, 2);
    }
};