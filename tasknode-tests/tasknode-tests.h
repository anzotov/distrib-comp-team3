#pragma once

#include "../tasknode/tasknode.h"

#include <QObject>
#include <QTest>
#include <functional>

struct TransportServiceMock final : TransportServiceBase
{
    TransportServiceMock(QObject *parent = nullptr) : TransportServiceBase(parent)
    {
        if (m_contructor)
            m_contructor(this);
    }
    ~TransportServiceMock()
    {
        if (m_destructor)
            m_destructor(this);
    }

    void sendHandshake(const PeerHandlerType &peerHandler, const Handshake &handshake) override final
    {
        ++m_sendHandshakeCount;
        if (m_sendHandshake)
            m_sendHandshake(this, peerHandler, handshake);
    }

    void sendCalcTask(const PeerHandlerType &peerHandler, const CalcTask &task) override final
    {
        ++m_sendCalcTaskCount;
        if (m_sendCalcTask)
            m_sendCalcTask(this, peerHandler, task);
    }
    void sendCalcResult(const PeerHandlerType &peerHandler, const CalcResult &result) override final
    {
        ++m_sendCalcResultCount;
        if (m_sendCalcResult)
            m_sendCalcResult(this, peerHandler, result);
    }
    void connectPeer(const QString &peerInfo) override final
    {
        ++m_connectPeerCount;
        if (m_connectPeer)
            m_connectPeer(this, peerInfo);
    }
    void disconnectPeer(const PeerHandlerType &peerHandler) override final
    {
        ++m_disconnectPeerCount;
        if (m_disconnectPeer)
            m_disconnectPeer(this, peerHandler);
    }
    void disconnectAllPeers() override final
    {
        ++m_disconnectAllPeersCount;
        if (m_disconnectAllPeers)
            m_disconnectAllPeers(this);
    }
    QList<PeerHandlerType> peers() const override final
    {
        ++m_peersCount;
        return m_peers(this);
    }
    void startListening() override final
    {
        ++m_startListeningCount;
        if (m_startListening)
            m_startListening(this);
    }
    void stopListening() override final
    {
        ++m_stopListeningCount;
        if (m_stopListening)
            m_stopListening(this);
    }

    std::function<void(TransportServiceMock *)> m_contructor;
    std::function<void(TransportServiceMock *)> m_destructor;
    std::function<void(TransportServiceMock *, const PeerHandlerType &, const Handshake &)> m_sendHandshake;
    int m_sendHandshakeCount = 0;
    std::function<void(TransportServiceMock *, const PeerHandlerType &, const CalcTask &)> m_sendCalcTask;
    int m_sendCalcTaskCount = 0;
    std::function<void(TransportServiceMock *, const PeerHandlerType &, const CalcResult &)> m_sendCalcResult;
    int m_sendCalcResultCount = 0;
    std::function<void(TransportServiceMock *, const QString &)> m_connectPeer;
    int m_connectPeerCount = 0;
    std::function<void(TransportServiceMock *, const PeerHandlerType &)> m_disconnectPeer;
    int m_disconnectPeerCount = 0;
    std::function<void(TransportServiceMock *)> m_disconnectAllPeers;
    int m_disconnectAllPeersCount = 0;
    std::function<QList<PeerHandlerType>(const TransportServiceMock *)> m_peers;
    mutable int m_peersCount = 0;
    std::function<void(TransportServiceMock *)> m_startListening;
    int m_startListeningCount = 0;
    std::function<void(TransportServiceMock *)> m_stopListening;
    int m_stopListeningCount = 0;
};

struct TaskProviderMock final : TaskProvider
{
    TaskProviderMock(QObject *parent = nullptr) : TaskProvider(parent)
    {
        if (m_constructor)
            m_constructor(this);
    }
    ~TaskProviderMock()
    {
        if (m_destructor)
            m_destructor(this);
    }
    void loadNextTask() override final
    {
        ++m_loadNextTaskCount;
        if (m_loadNextTask)
            m_loadNextTask(this);
    }
    void formatResult(const CalcResult &result) override final
    {
        ++m_formatResultCount;
        if (m_formatResult)
            m_formatResult(this, result);
    }

    std::function<void(TaskProviderMock *)> m_constructor;
    std::function<void(TaskProviderMock *)> m_destructor;
    std::function<void(TaskProviderMock *)> m_loadNextTask;
    int m_loadNextTaskCount = 0;
    std::function<void(TaskProviderMock *, const CalcResult &result)> m_formatResult;
    int m_formatResultCount = 0;
};

class TaskNodeTests : public QObject
{
    Q_OBJECT

private slots:
    void PositiveTest()
    {
        QString test_peerInfo = "127.0.0.1:3333";
        const auto test_task = CalcTask("sin(x)", {"0", "1"}, true);
        const auto test_result = CalcResult({"1", "2"}, true);
        TransportServiceBase::PeerHandlerType test_peerHandler = "1234";
        Handshake test_handshake(Handshake::PeerType::CompNode, "1000", "{12345-4-5-6}");
        auto test_taskProviderMock = new TaskProviderMock;
        test_taskProviderMock->m_loadNextTask =
            [&](TaskProviderMock *self)
        {
            emit self->taskLoadDone(test_task);
        };
        test_taskProviderMock->m_formatResult =
            [&](TaskProviderMock *self, const CalcResult &result)
        {
            QCOMPARE(result, test_result);
            emit self->resultFormatDone();
        };

        auto test_transportServiceMock = new TransportServiceMock;
        test_transportServiceMock->m_connectPeer =
            [&](TransportServiceMock *self, const QString &peerInfo)
        {
            QCOMPARE(peerInfo, test_peerInfo);
            emit self->newPeer(test_peerHandler, test_peerInfo, true);
        };
        test_transportServiceMock->m_sendHandshake =
            [&](TransportServiceMock *self, const TransportServiceBase::PeerHandlerType &peerHandler, const Handshake &handshake)
        {
            QCOMPARE(peerHandler, test_peerHandler);
            QCOMPARE(handshake.peerType, Handshake::PeerType::TaskNode);
            QVERIFY(!handshake.uuid.isEmpty());
            emit self->receivedHandshake(peerHandler, test_handshake);
        };
        test_transportServiceMock->m_sendCalcTask =
            [&](TransportServiceMock *self, const TransportServiceBase::PeerHandlerType &peerHandler, const CalcTask &task)
        {
            QCOMPARE(peerHandler, test_peerHandler);
            QCOMPARE(task, test_task);
            emit self->receivedCalcResult(peerHandler, test_result);
        };
        test_transportServiceMock->m_disconnectPeer =
            [&](TransportServiceMock *self, const TransportServiceBase::PeerHandlerType &peerHandler)
        {
            QCOMPARE(peerHandler, test_peerHandler);
            emit self->peerDiconnected(peerHandler);
        };

        TaskNode taskNode(test_transportServiceMock, test_taskProviderMock, test_peerInfo, 10, 1);

        int test_stoppedCount = 0;
        QObject::connect(&taskNode, &TaskNode::stopped, this, [&](bool success)
                         {
                            ++test_stoppedCount;
                            QVERIFY(success); });

        taskNode.start();
        QTRY_COMPARE(test_transportServiceMock->m_connectPeerCount, 1);
        QTRY_COMPARE(test_transportServiceMock->m_sendHandshakeCount, 1);
        QTRY_COMPARE(test_taskProviderMock->m_loadNextTaskCount, 1);
        QTRY_COMPARE(test_transportServiceMock->m_sendCalcTaskCount, 1);
        QTRY_COMPARE(test_taskProviderMock->m_formatResultCount, 1);
        QTRY_COMPARE(test_transportServiceMock->m_disconnectPeerCount, 1);
        QTRY_COMPARE(test_stoppedCount, 1);
    }
    void LoadErrorTest()
    {
        QString test_peerInfo = "127.0.0.1:3333";
        const auto test_task = CalcTask("sin(x)", {"0", "1"}, true);
        const auto test_result = CalcResult({"1", "2"}, true);
        TransportServiceBase::PeerHandlerType test_peerHandler = "1234";
        Handshake test_handshake(Handshake::PeerType::CompNode, "1000", "{12345-4-5-6}");
        auto test_taskProviderMock = new TaskProviderMock;
        test_taskProviderMock->m_loadNextTask =
            [&](TaskProviderMock *self)
        {
            emit self->taskLoadError();
        };

        auto test_transportServiceMock = new TransportServiceMock;
        TaskNode taskNode(test_transportServiceMock, test_taskProviderMock, test_peerInfo, 10, 1);

        int test_stoppedCount = 0;
        QObject::connect(&taskNode, &TaskNode::stopped, this, [&](bool success)
                         {
                            ++test_stoppedCount;
                            QVERIFY(!success); });

        taskNode.start();
        QTRY_COMPARE(test_stoppedCount, 1);
        QTRY_COMPARE(test_taskProviderMock->m_loadNextTaskCount, 1);
        QTRY_COMPARE(test_transportServiceMock->m_disconnectAllPeersCount, 1);
    }
    void NoTasksTest()
    {
        QString test_peerInfo = "127.0.0.1:3333";
        const auto test_task = CalcTask("sin(x)", {"0", "1"}, true);
        const auto test_result = CalcResult({"1", "2"}, true);
        TransportServiceBase::PeerHandlerType test_peerHandler = "1234";
        Handshake test_handshake(Handshake::PeerType::CompNode, "1000", "{12345-4-5-6}");
        auto test_taskProviderMock = new TaskProviderMock;
        test_taskProviderMock->m_loadNextTask =
            [&](TaskProviderMock *self)
        {
            emit self->noTasksAvailable();
        };

        auto test_transportServiceMock = new TransportServiceMock;
        TaskNode taskNode(test_transportServiceMock, test_taskProviderMock, test_peerInfo, 10, 1);

        int test_stoppedCount = 0;
        QObject::connect(&taskNode, &TaskNode::stopped, this, [&](bool success)
                         {
                            ++test_stoppedCount;
                            QVERIFY(!success); });

        taskNode.start();
        QTRY_COMPARE(test_stoppedCount, 1);
        QTRY_COMPARE(test_taskProviderMock->m_loadNextTaskCount, 1);
        QTRY_COMPARE(test_transportServiceMock->m_disconnectAllPeersCount, 1);
    }
    void FormatErrorTest()
    {
        QString test_peerInfo = "127.0.0.1:3333";
        const auto test_task = CalcTask("sin(x)", {"0", "1"}, true);
        const auto test_result = CalcResult({"1", "2"}, true);
        TransportServiceBase::PeerHandlerType test_peerHandler = "1234";
        Handshake test_handshake(Handshake::PeerType::CompNode, "1000", "{12345-4-5-6}");
        auto test_taskProviderMock = new TaskProviderMock;
        test_taskProviderMock->m_loadNextTask =
            [&](TaskProviderMock *self)
        {
            emit self->taskLoadDone(test_task);
        };
        test_taskProviderMock->m_formatResult =
            [&](TaskProviderMock *self, const CalcResult &result)
        {
            QCOMPARE(result, test_result);
            emit self->resultFormatError(result);
        };

        auto test_transportServiceMock = new TransportServiceMock;
        test_transportServiceMock->m_connectPeer =
            [&](TransportServiceMock *self, const QString &peerInfo)
        {
            QCOMPARE(peerInfo, test_peerInfo);
            emit self->newPeer(test_peerHandler, test_peerInfo, true);
        };
        test_transportServiceMock->m_sendHandshake =
            [&](TransportServiceMock *self, const TransportServiceBase::PeerHandlerType &peerHandler, const Handshake &handshake)
        {
            QCOMPARE(peerHandler, test_peerHandler);
            QCOMPARE(handshake.peerType, Handshake::PeerType::TaskNode);
            QVERIFY(!handshake.uuid.isEmpty());
            emit self->receivedHandshake(peerHandler, test_handshake);
        };
        test_transportServiceMock->m_sendCalcTask =
            [&](TransportServiceMock *self, const TransportServiceBase::PeerHandlerType &peerHandler, const CalcTask &task)
        {
            QCOMPARE(peerHandler, test_peerHandler);
            QCOMPARE(task, test_task);
            emit self->receivedCalcResult(peerHandler, test_result);
        };
        test_transportServiceMock->m_disconnectPeer =
            [&](TransportServiceMock *self, const TransportServiceBase::PeerHandlerType &peerHandler)
        {
            QCOMPARE(peerHandler, test_peerHandler);
            emit self->peerDiconnected(peerHandler);
        };

        TaskNode taskNode(test_transportServiceMock, test_taskProviderMock, test_peerInfo, 10, 1);

        int test_stoppedCount = 0;
        QObject::connect(&taskNode, &TaskNode::stopped, this, [&](bool success)
                         {
                            ++test_stoppedCount;
                            QVERIFY(!success); });

        taskNode.start();
        QTRY_COMPARE(test_transportServiceMock->m_connectPeerCount, 1);
        QTRY_COMPARE(test_transportServiceMock->m_sendHandshakeCount, 1);
        QTRY_COMPARE(test_taskProviderMock->m_loadNextTaskCount, 1);
        QTRY_COMPARE(test_transportServiceMock->m_sendCalcTaskCount, 1);
        QTRY_COMPARE(test_taskProviderMock->m_formatResultCount, 1);
        QTRY_COMPARE(test_transportServiceMock->m_disconnectPeerCount, 1);
        QTRY_COMPARE(test_transportServiceMock->m_disconnectAllPeersCount, 1);
        QTRY_COMPARE(test_stoppedCount, 1);
    }
    void ConnectErrorTest()
    {
        QString test_peerInfo = "127.0.0.1:3333";
        const auto test_task = CalcTask("sin(x)", {"0", "1"}, true);
        const auto test_result = CalcResult({"1", "2"}, true);
        TransportServiceBase::PeerHandlerType test_peerHandler = "1234";
        Handshake test_handshake(Handshake::PeerType::CompNode, "1000", "{12345-4-5-6}");
        auto test_taskProviderMock = new TaskProviderMock;
        test_taskProviderMock->m_loadNextTask =
            [&](TaskProviderMock *self)
        {
            emit self->taskLoadDone(test_task);
        };
        test_taskProviderMock->m_formatResult =
            [&](TaskProviderMock *self, const CalcResult &result)
        {
            QCOMPARE(result, test_result);
            emit self->resultFormatError(result);
        };

        auto test_transportServiceMock = new TransportServiceMock;
        test_transportServiceMock->m_connectPeer =
            [&](TransportServiceMock *self, const QString &peerInfo)
        {
            QCOMPARE(peerInfo, test_peerInfo);
            emit self->connectError(peerInfo);
        };

        TaskNode taskNode(test_transportServiceMock, test_taskProviderMock, test_peerInfo, 10, 1);

        int test_stoppedCount = 0;
        QObject::connect(&taskNode, &TaskNode::stopped, this, [&](bool success)
                         {
                            ++test_stoppedCount;
                            QVERIFY(!success); });

        taskNode.start();
        QTRY_COMPARE(test_transportServiceMock->m_connectPeerCount, 1);
        QTRY_COMPARE(test_taskProviderMock->m_loadNextTaskCount, 1);
        QTRY_COMPARE(test_transportServiceMock->m_disconnectAllPeersCount, 1);
        QTRY_COMPARE(test_stoppedCount, 1);
    }
};