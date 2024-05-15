#pragma once

#include "networkMock.h"
#include "../common/transportService.h"
#include "../common/jsonSerializer.h"
#include "../common/compressor.h"
#include "../compnode/peerService.h"
#include "../compnode/chunkerService.h"
#include "../compnode/compnode.h"
#include "../compnode/jsCalculatorService.h"
#include "../tasknode-tests/taskProviderMock.h"
#include "../tasknode/tasknode.h"
#include "../common-tests/qtest-toString.h"

#include <QTest>

using namespace std::literals;

class NoNetworkTestCase : public QObject
{
    Q_OBJECT
private slots:
    void OneTaskNodeToOneCompNodeTest()
    {
        NetworkMock network;
        auto test_task = CalcTask("2+x**3", {"0", "1", "2"}, true);
        auto test_result = CalcResult({"2", "3", "10"}, true);
        auto test_taskProvider = new TaskProviderMock;
        test_taskProvider->m_loadNextTask =
            [&](TaskProviderMock *self)
        {
            emit self->taskLoadDone(test_task);
        };
        test_taskProvider->m_formatResult =
            [&](TaskProviderMock *self, const CalcResult &result)
        {
            QCOMPARE(result, test_result);
            emit self->resultFormatDone();
        };
        CompNode test_compnode(new PeerService(new TransportService(
                                                   network.createTransportLayer("127.0.0.1:3333"),
                                                   new JsonSerializer,
                                                   new Compressor),
                                               network.createDiscoveryService(1s),
                                               DiscoveryData("{1234-5-6}", {"127.0.0.1:3333"})),
                               new ChunkerService(new JsCalculatorService()));
        TaskNode test_tasknode(new TransportService(
                                   network.createTransportLayer("127.0.0.1:5555"),
                                   new JsonSerializer,
                                   new Compressor),
                               test_taskProvider,
                               "127.0.0.1:3333");
        int test_tasknodeStoppedCount = 0;
        QObject::connect(&test_tasknode, &TaskNode::stopped, this,
                         [&](bool success)
                         {
                             ++test_tasknodeStoppedCount;
                             QVERIFY(success);
                         });
        test_compnode.start();
        test_tasknode.start();
        QTRY_COMPARE(test_taskProvider->m_loadNextTaskCount, 1);
        QTRY_COMPARE(test_taskProvider->m_formatResultCount, 1);
        QTRY_COMPARE(test_tasknodeStoppedCount, 1);
    }
    void OneTaskNodeToTwoCompNodesTest()
    {
        NetworkMock network;
        auto test_task = CalcTask("2+x**3", {"0", "1", "2"}, true);
        auto test_result = CalcResult({"2", "3", "10"}, true);
        auto test_taskProvider = new TaskProviderMock;
        test_taskProvider->m_loadNextTask =
            [&](TaskProviderMock *self)
        {
            emit self->taskLoadDone(test_task);
        };
        test_taskProvider->m_formatResult =
            [&](TaskProviderMock *self, const CalcResult &result)
        {
            QCOMPARE(result, test_result);
            emit self->resultFormatDone();
        };
        CompNode test_compnode1(new PeerService(new TransportService(
                                                    network.createTransportLayer("127.0.0.1:3333"),
                                                    new JsonSerializer,
                                                    new Compressor),
                                                network.createDiscoveryService(1s),
                                                DiscoveryData("{1234-5-6}", {"127.0.0.1:3333"})),
                                new ChunkerService(new JsCalculatorService()));
        CompNode test_compnode2(new PeerService(new TransportService(
                                                    network.createTransportLayer("127.0.0.1:4444"),
                                                    new JsonSerializer,
                                                    new Compressor),
                                                network.createDiscoveryService(1s),
                                                DiscoveryData("{1234-5-7}", {"127.0.0.1:4444"})),
                                new ChunkerService(new JsCalculatorService()));
        TaskNode test_tasknode(new TransportService(
                                   network.createTransportLayer("127.0.0.1:5555"),
                                   new JsonSerializer,
                                   new Compressor),
                               test_taskProvider,
                               "127.0.0.1:3333");
        int test_tasknodeStoppedCount = 0;
        QObject::connect(&test_tasknode, &TaskNode::stopped, this,
                         [&](bool success)
                         {
                             ++test_tasknodeStoppedCount;
                             QVERIFY(success);
                         });
        test_compnode1.start();
        test_compnode2.start();
        QTest::qWait(5000);
        test_tasknode.start();
        QTRY_COMPARE(test_taskProvider->m_loadNextTaskCount, 1);
        QTRY_COMPARE(test_taskProvider->m_formatResultCount, 1);
        QTRY_COMPARE(test_tasknodeStoppedCount, 1);
    }
};