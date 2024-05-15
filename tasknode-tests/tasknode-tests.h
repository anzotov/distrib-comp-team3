#pragma once

#include "../tasknode/tasknode.h"
#include "../common-tests/transportServiceMock.h"
#include "taskProviderMock.h"
#include "../common-tests/qtest-toString.h"

#include <QObject>
#include <QTest>

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
    void DisconnectTest()
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
        QTRY_COMPARE(test_stoppedCount, 1);
    }
    void StopTest()
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
            [&](TransportServiceMock *, const TransportServiceBase::PeerHandlerType &peerHandler, const CalcTask &task)
        {
            QCOMPARE(peerHandler, test_peerHandler);
            QCOMPARE(task, test_task);
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
        taskNode.stop();
        QTRY_COMPARE(test_transportServiceMock->m_disconnectAllPeersCount, 1);
        QTRY_COMPARE(test_stoppedCount, 1);
    }
    void RestartTest()
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
        taskNode.start();
        QTRY_COMPARE(test_transportServiceMock->m_connectPeerCount, 2);
        QTRY_COMPARE(test_transportServiceMock->m_sendHandshakeCount, 2);
        QTRY_COMPARE(test_taskProviderMock->m_loadNextTaskCount, 2);
        QTRY_COMPARE(test_transportServiceMock->m_sendCalcTaskCount, 2);
        QTRY_COMPARE(test_taskProviderMock->m_formatResultCount, 2);
        QTRY_COMPARE(test_transportServiceMock->m_disconnectPeerCount, 2);
        QTRY_COMPARE(test_stoppedCount, 2);
    }
};