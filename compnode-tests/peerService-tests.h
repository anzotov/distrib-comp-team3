#pragma once


#include "../compnode/peerService.h"
#include "../common-tests/transportServiceMock.h"
#include "discoveryServiceMock.h"
#include "../common-tests/qtest-toString.h"

#include <QObject>
#include <QTest>
#include <QUuid>

class PeerServiceTestCase final : public QObject
{
    Q_OBJECT

private slots:
    void InboundConnectionTest()
    {
        auto test_transportServiceMock = new TransportServiceMock;
        auto test_discoveryServiceMock = new DiscoveryServiceMock;

        QString test_selfCompPower = "1000";
        DiscoveryData test_selfDiscoveryData(QUuid::createUuid().toString(), {"127.0.0.1:1234"});

        QString test_peerNetworkInfo = "192.168.0.1";
        TransportServiceBase::PeerHandlerType test_peerTransportHandler = "111";
        Handshake test_peerHandshake(Handshake::PeerType::CompNode, "999", "{123-1-1-1}");

        CalcTask test_task("sin(x)", {"3", "4"}, false);
        CalcResult test_result({"5", "6"}, false);

        test_transportServiceMock->m_sendHandshake = [&](TransportServiceMock *, const PeerHandlerType &peerHandler, const Handshake &handshake)
        {
            QCOMPARE(peerHandler, test_peerTransportHandler);
            QCOMPARE(handshake.peerType, Handshake::PeerType::CompNode);
            QCOMPARE(handshake.uuid, test_selfDiscoveryData.uuid);
            QCOMPARE(handshake.compPower, test_selfCompPower);
        };
        test_transportServiceMock->m_sendCalcResult = [&](TransportServiceMock *, const PeerHandlerType &peerHandler, const CalcResult &result)
        {
            QCOMPARE(peerHandler, test_peerTransportHandler);
            QCOMPARE(result, test_result);
        };
        test_transportServiceMock->m_sendCalcTask = [&](TransportServiceMock *, const PeerHandlerType &peerHandler, const CalcTask &task)
        {
            QCOMPARE(peerHandler, test_peerTransportHandler);
            QCOMPARE(task, test_task);
        };

        test_discoveryServiceMock->m_startMulticast = [&](DiscoveryServiceMock *, const DiscoveryData &discoveryData)
        {
            QCOMPARE(discoveryData, test_selfDiscoveryData);
        };

        PeerService test_peerService(test_transportServiceMock, test_discoveryServiceMock, test_selfDiscoveryData);

        int test_peersChangedCount = 0;
        QObject::connect(&test_peerService, &PeerServiceBase::peersChanged, this,
                         [&](QList<PeerInfo> peers)
                         {
                             ++test_peersChangedCount;
                             if (test_peersChangedCount == 1)
                             {
                                 QCOMPARE(peers.size(), 1);
                                 QCOMPARE(peers.at(0).peerHandler, test_peerHandshake.uuid);
                                 QCOMPARE(peers.at(0).compPower, test_peerHandshake.compPower);
                             }
                             else
                             {
                                 QCOMPARE(peers.size(), 0);
                             }
                         });

        int test_receivedCalcTaskCount = 0;
        QObject::connect(&test_peerService, &PeerServiceBase::receivedCalcTask, this,
                         [&](const PeerHandlerType peerHandler, const CalcTask task)
                         {
                             ++test_receivedCalcTaskCount;
                             QCOMPARE(peerHandler, test_peerHandshake.uuid);
                             QCOMPARE(task, test_task);
                         });
        int test_receivedCalcResultCount = 0;
        QObject::connect(&test_peerService, &PeerServiceBase::receivedCalcResult, this,
                         [&](const PeerHandlerType peerHandler, const CalcResult result)
                         {
                             ++test_receivedCalcResultCount;
                             QCOMPARE(peerHandler, test_peerHandshake.uuid);
                             QCOMPARE(result, test_result);
                         });

        test_peerService.start(test_selfCompPower);
        QTRY_COMPARE(test_transportServiceMock->m_startListeningCount, 1);
        QTRY_COMPARE(test_discoveryServiceMock->m_startListeningCount, 1);
        QTRY_COMPARE(test_discoveryServiceMock->m_startMulticastCount, 1);

        emit test_transportServiceMock->newPeer(test_peerTransportHandler, test_peerNetworkInfo, false);
        QTRY_COMPARE(test_transportServiceMock->m_sendHandshakeCount, 1);

        emit test_transportServiceMock->receivedHandshake(test_peerTransportHandler, test_peerHandshake);
        QTRY_COMPARE(test_peersChangedCount, 1);

        emit test_transportServiceMock->receivedCalcTask(test_peerTransportHandler, test_task);
        QTRY_COMPARE(test_receivedCalcTaskCount, 1);

        test_peerService.sendCalcResult(test_peerHandshake.uuid, test_result);
        QTRY_COMPARE(test_transportServiceMock->m_sendCalcResultCount, 1);

        emit test_transportServiceMock->receivedCalcResult(test_peerTransportHandler, test_result);
        QTRY_COMPARE(test_receivedCalcResultCount, 1);

        test_peerService.sendCalcTask(test_peerHandshake.uuid, test_task);
        QTRY_COMPARE(test_transportServiceMock->m_sendCalcTaskCount, 1);

        emit test_transportServiceMock->peerDiconnected(test_peerTransportHandler);
        QTRY_COMPARE(test_peersChangedCount, 2);
    }

    void OutboundConnectionTest()
    {
        auto test_transportServiceMock = new TransportServiceMock;
        auto test_discoveryServiceMock = new DiscoveryServiceMock;

        QString test_selfCompPower = "1000";
        DiscoveryData test_selfDiscoveryData(QUuid::createUuid().toString(), {"127.0.0.1:1234"});

        QString test_peerNetworkInfo = "192.168.0.1";
        TransportServiceBase::PeerHandlerType test_peerTransportHandler = "111";
        Handshake test_peerHandshake(Handshake::PeerType::CompNode, "999", "{123-1-1-1}");
        QList<DiscoveryData> test_peers = {DiscoveryData(test_peerHandshake.uuid, {test_peerNetworkInfo})};

        CalcTask test_task("sin(x)", {"3", "4"}, false);
        CalcResult test_result({"5", "6"}, false);

        test_transportServiceMock->m_sendHandshake = [&](TransportServiceMock *, const PeerHandlerType &peerHandler, const Handshake &handshake)
        {
            QCOMPARE(peerHandler, test_peerTransportHandler);
            QCOMPARE(handshake.peerType, Handshake::PeerType::CompNode);
            QCOMPARE(handshake.uuid, test_selfDiscoveryData.uuid);
            QCOMPARE(handshake.compPower, test_selfCompPower);
        };
        test_transportServiceMock->m_sendCalcResult =
            [&](TransportServiceMock *, const PeerHandlerType &peerHandler, const CalcResult &result)
        {
            QCOMPARE(peerHandler, test_peerTransportHandler);
            QCOMPARE(result, test_result);
        };
        test_transportServiceMock->m_sendCalcTask =
            [&](TransportServiceMock *, const PeerHandlerType &peerHandler, const CalcTask &task)
        {
            QCOMPARE(peerHandler, test_peerTransportHandler);
            QCOMPARE(task, test_task);
        };

        test_transportServiceMock->m_connectPeer =
            [&](TransportServiceMock *, const QString &peerInfo)
        {
            QCOMPARE(peerInfo, test_peerNetworkInfo);
        };

        test_discoveryServiceMock->m_startMulticast = [&](DiscoveryServiceMock *, const DiscoveryData &discoveryData)
        {
            QCOMPARE(discoveryData, test_selfDiscoveryData);
        };

        PeerService test_peerService(test_transportServiceMock, test_discoveryServiceMock, test_selfDiscoveryData);

        int test_peersChangedCount = 0;
        QObject::connect(&test_peerService, &PeerServiceBase::peersChanged, this,
                         [&](QList<PeerInfo> peers)
                         {
                             ++test_peersChangedCount;
                             if (test_peersChangedCount == 1)
                             {
                                 QCOMPARE(peers.size(), 1);
                                 QCOMPARE(peers.at(0).peerHandler, test_peerHandshake.uuid);
                                 QCOMPARE(peers.at(0).compPower, test_peerHandshake.compPower);
                             }
                             else
                             {
                                 QCOMPARE(peers.size(), 0);
                             }
                         });

        int test_receivedCalcTaskCount = 0;
        QObject::connect(&test_peerService, &PeerServiceBase::receivedCalcTask, this,
                         [&](const PeerHandlerType peerHandler, const CalcTask task)
                         {
                             ++test_receivedCalcTaskCount;
                             QCOMPARE(peerHandler, test_peerHandshake.uuid);
                             QCOMPARE(task, test_task);
                         });
        int test_receivedCalcResultCount = 0;
        QObject::connect(&test_peerService, &PeerServiceBase::receivedCalcResult, this,
                         [&](const PeerHandlerType peerHandler, const CalcResult result)
                         {
                             ++test_receivedCalcResultCount;
                             QCOMPARE(peerHandler, test_peerHandshake.uuid);
                             QCOMPARE(result, test_result);
                         });

        test_peerService.start(test_selfCompPower);
        QTRY_COMPARE(test_transportServiceMock->m_startListeningCount, 1);
        QTRY_COMPARE(test_discoveryServiceMock->m_startListeningCount, 1);
        QTRY_COMPARE(test_discoveryServiceMock->m_startMulticastCount, 1);

        emit test_discoveryServiceMock->peersChanged(test_peers);
        QTRY_COMPARE(test_transportServiceMock->m_connectPeerCount, 1);

        emit test_discoveryServiceMock->peersChanged(test_peers);
        QTRY_COMPARE(test_transportServiceMock->m_connectPeerCount, 1);

        emit test_transportServiceMock->newPeer(test_peerTransportHandler, test_peerNetworkInfo, true);
        QTRY_COMPARE(test_transportServiceMock->m_sendHandshakeCount, 1);

        emit test_discoveryServiceMock->peersChanged(test_peers);
        QTRY_COMPARE(test_transportServiceMock->m_connectPeerCount, 1);

        emit test_transportServiceMock->receivedHandshake(test_peerTransportHandler, test_peerHandshake);
        QTRY_COMPARE(test_peersChangedCount, 1);

        emit test_transportServiceMock->receivedCalcTask(test_peerTransportHandler, test_task);
        QTRY_COMPARE(test_receivedCalcTaskCount, 1);

        test_peerService.sendCalcResult(test_peerHandshake.uuid, test_result);
        QTRY_COMPARE(test_transportServiceMock->m_sendCalcResultCount, 1);

        emit test_transportServiceMock->receivedCalcResult(test_peerTransportHandler, test_result);
        QTRY_COMPARE(test_receivedCalcResultCount, 1);

        test_peerService.sendCalcTask(test_peerHandshake.uuid, test_task);
        QTRY_COMPARE(test_transportServiceMock->m_sendCalcTaskCount, 1);

        emit test_transportServiceMock->peerDiconnected(test_peerTransportHandler);
        QTRY_COMPARE(test_peersChangedCount, 2);

        emit test_discoveryServiceMock->peersChanged(test_peers);
        QTRY_COMPARE(test_transportServiceMock->m_connectPeerCount, 2);
    }

    void TaskNodeConnectionTest()
    {
        auto test_transportServiceMock = new TransportServiceMock;
        auto test_discoveryServiceMock = new DiscoveryServiceMock;

        QString test_selfCompPower = "1000";
        DiscoveryData test_selfDiscoveryData(QUuid::createUuid().toString(), {"127.0.0.1:1234"});

        QString test_taskNodeNetworkInfo = "192.168.0.1";
        TransportServiceBase::PeerHandlerType test_taskNodeTransportHandler = "111";
        Handshake test_taskNodeHandshake(Handshake::PeerType::TaskNode, "", "{123-1-1-1}");

        CalcTask test_task("sin(x)", {"3", "4"}, true);
        CalcResult test_result({"5", "6"}, true);

        test_transportServiceMock->m_sendHandshake = [&](TransportServiceMock *, const PeerHandlerType &peerHandler, const Handshake &handshake)
        {
            QCOMPARE(peerHandler, test_taskNodeTransportHandler);
            QCOMPARE(handshake.peerType, Handshake::PeerType::CompNode);
            QCOMPARE(handshake.uuid, test_selfDiscoveryData.uuid);
            QCOMPARE(handshake.compPower, test_selfCompPower);
        };
        test_transportServiceMock->m_sendCalcResult = [&](TransportServiceMock *, const PeerHandlerType &peerHandler, const CalcResult &result)
        {
            QCOMPARE(peerHandler, test_taskNodeTransportHandler);
            QCOMPARE(result, test_result);
        };
        test_transportServiceMock->m_sendCalcTask = [&](TransportServiceMock *, const PeerHandlerType &peerHandler, const CalcTask &task)
        {
            QCOMPARE(peerHandler, test_taskNodeTransportHandler);
            QCOMPARE(task, test_task);
        };

        PeerService test_peerService(test_transportServiceMock, test_discoveryServiceMock, test_selfDiscoveryData);

        int test_peersChangedCount = 0;
        QObject::connect(&test_peerService, &PeerServiceBase::peersChanged, this,
                         [&](QList<PeerInfo>)
                         {
                             ++test_peersChangedCount;
                         });
        int test_taskNodeDisconnectedCount = 0;
        QObject::connect(&test_peerService, &PeerServiceBase::taskNodeDisconnected, this,
                         [&](const PeerHandlerType peerHandler)
                         {
                             ++test_taskNodeDisconnectedCount;
                             QCOMPARE(peerHandler, test_taskNodeHandshake.uuid);
                         });

        int test_receivedCalcTaskCount = 0;
        QObject::connect(&test_peerService, &PeerServiceBase::receivedCalcTask, this,
                         [&](const PeerHandlerType peerHandler, const CalcTask task)
                         {
                             ++test_receivedCalcTaskCount;
                             QCOMPARE(peerHandler, test_taskNodeHandshake.uuid);
                             QCOMPARE(task, test_task);
                         });
        int test_receivedCalcResultCount = 0;
        QObject::connect(&test_peerService, &PeerServiceBase::receivedCalcResult, this,
                         [&](const PeerHandlerType peerHandler, const CalcResult result)
                         {
                             ++test_receivedCalcResultCount;
                             QCOMPARE(peerHandler, test_taskNodeHandshake.uuid);
                             QCOMPARE(result, test_result);
                         });

        test_peerService.start(test_selfCompPower);
        QTRY_COMPARE(test_transportServiceMock->m_startListeningCount, 1);
        QTRY_COMPARE(test_discoveryServiceMock->m_startListeningCount, 1);
        QTRY_COMPARE(test_discoveryServiceMock->m_startMulticastCount, 1);

        emit test_transportServiceMock->newPeer(test_taskNodeTransportHandler, test_taskNodeNetworkInfo, false);
        QTRY_COMPARE(test_transportServiceMock->m_sendHandshakeCount, 1);

        emit test_transportServiceMock->receivedHandshake(test_taskNodeTransportHandler, test_taskNodeHandshake);
        QTRY_COMPARE(test_peersChangedCount, 0);

        emit test_transportServiceMock->receivedCalcTask(test_taskNodeTransportHandler, test_task);
        QTRY_COMPARE(test_receivedCalcTaskCount, 1);

        test_peerService.sendCalcResult(test_taskNodeHandshake.uuid, test_result);
        QTRY_COMPARE(test_transportServiceMock->m_sendCalcResultCount, 1);

        emit test_transportServiceMock->receivedCalcResult(test_taskNodeTransportHandler, test_result);
        QTRY_COMPARE(test_receivedCalcResultCount, 1);

        test_peerService.sendCalcTask(test_taskNodeHandshake.uuid, test_task);
        QTRY_COMPARE(test_transportServiceMock->m_sendCalcTaskCount, 1);

        emit test_transportServiceMock->peerDiconnected(test_taskNodeTransportHandler);
        QTRY_COMPARE(test_taskNodeDisconnectedCount, 1);
        QTRY_COMPARE(test_peersChangedCount, 0);
    }
};