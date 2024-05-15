#pragma once

#include "../common/jsonSerializer.h"
#include "encoderMock.h"
#include "transportLayerMock.h"
#include "../common-tests/qtest-toString.h"

#include <QObject>
#include <QTest>

class TransportServiceTestCase : public QObject
{
    Q_OBJECT
private slots:
    void Test1()
    {
        QString test_peerInfo = "127.0.0.1:3333";
        TransportLayerBase::PeerHandlerType test_peerHandler = "123";
        Handshake test_handshake(Handshake::PeerType::CompNode, "1000", "{1234-5-6-7}");
        CalcTask test_task("sin(x)", {"5", "6"}, true);
        CalcResult test_result({"1", "2"}, true);
        auto test_transportLayerMock = new TransportLayerMock;
        test_transportLayerMock->m_openConnection =
            [&](TransportLayerMock *self, const QString &peerInfo)
        {
            QCOMPARE(peerInfo, test_peerInfo);
            emit self->newConnection(test_peerHandler, test_peerInfo, true);
        };
        test_transportLayerMock->m_sendData = [&](TransportLayerMock *self, const TransportLayerMock::PeerHandlerType &peerHandler, const QByteArray &data)
        {
            emit self->dataReceived(peerHandler, data);
        };
        test_transportLayerMock->m_closeConnection =
            [&](TransportLayerMock *self, const TransportLayerBase::PeerHandlerType &peerHandler)
        {
            QCOMPARE(peerHandler, test_peerHandler);
            emit self->connectionClosed(peerHandler);
        };

        auto test_encoder = new EncoderMock;
        test_encoder->m_encode =
            [&](const EncoderMock *, const QByteArray &data)
        {
            return data.toBase64();
        };
        test_encoder->m_decode =
            [&](const EncoderMock *, const QByteArray &data)
        {
            return QByteArray::fromBase64(data);
        };

        TransportService test_transportService(test_transportLayerMock, new JsonSerializer, test_encoder);
        int test_newPeerCount = 0;
        QObject::connect(&test_transportService, &TransportService::newPeer,
                         [&](const TransportServiceBase::PeerHandlerType peerHandler, const QString peerInfo, bool outgoing)
                         {
                             ++test_newPeerCount;
                             QCOMPARE(peerHandler, test_peerHandler);
                             QCOMPARE(peerInfo, test_peerInfo);
                             QVERIFY(outgoing);
                         });

        int test_receivedHandshakeCount = 0;
        QObject::connect(&test_transportService, &TransportService::receivedHandshake, this,
                         [&](const TransportServiceBase::PeerHandlerType peerHandler, const Handshake handshake)
                         {
                             ++test_receivedHandshakeCount;
                             QCOMPARE(peerHandler, test_peerHandler);
                             QCOMPARE(handshake, test_handshake);
                         });
        int test_receivedCalcTaskCount = 0;
        QObject::connect(&test_transportService, &TransportService::receivedCalcTask, this,
                         [&](const TransportServiceBase::PeerHandlerType peerHandler, const CalcTask task)
                         {
                             ++test_receivedCalcTaskCount;
                             QCOMPARE(peerHandler, test_peerHandler);
                             QCOMPARE(task, test_task);
                         });
        int test_receivedCalcResultCount = 0;
        QObject::connect(&test_transportService, &TransportService::receivedCalcResult, this,
                         [&](const TransportServiceBase::PeerHandlerType peerHandler, const CalcResult result)
                         {
                             ++test_receivedCalcResultCount;
                             QCOMPARE(peerHandler, test_peerHandler);
                             QCOMPARE(result, test_result);
                         });
        int test_peerDiconnectedCount = 0;
        QObject::connect(&test_transportService, &TransportService::peerDiconnected, this,
                         [&](const TransportService::PeerHandlerType peerHandler)
                         {
                             ++test_peerDiconnectedCount;
                             QCOMPARE(peerHandler, test_peerHandler);
                         });

        test_transportService.startListening();
        QTRY_COMPARE(test_transportLayerMock->m_startListeningCount, 1);

        test_transportService.connectPeer(test_peerInfo);
        QTRY_COMPARE(test_transportLayerMock->m_openConnectionCount, 1);
        QTRY_COMPARE(test_newPeerCount, 1);
        QCOMPARE(test_transportService.peers(), {test_peerHandler});

        test_transportService.sendHandshake(test_peerHandler, test_handshake);
        QTRY_COMPARE(test_transportLayerMock->m_sendDataCount, 1);
        QTRY_COMPARE(test_encoder->m_encodeCount, 1);
        QTRY_COMPARE(test_receivedHandshakeCount, 1);
        QTRY_COMPARE(test_encoder->m_decodeCount, 1);

        test_transportService.sendCalcTask(test_peerHandler, test_task);
        QTRY_COMPARE(test_transportLayerMock->m_sendDataCount, 2);
        QTRY_COMPARE(test_encoder->m_encodeCount, 2);
        QTRY_COMPARE(test_receivedCalcTaskCount, 1);
        QTRY_COMPARE(test_encoder->m_decodeCount, 2);

        test_transportService.sendCalcResult(test_peerHandler, test_result);
        QTRY_COMPARE(test_transportLayerMock->m_sendDataCount, 3);
        QTRY_COMPARE(test_encoder->m_encodeCount, 3);
        QTRY_COMPARE(test_receivedCalcResultCount, 1);
        QTRY_COMPARE(test_encoder->m_decodeCount, 3);

        test_transportService.disconnectPeer(test_peerHandler);
        QTRY_COMPARE(test_transportLayerMock->m_closeConnectionCount, 1);
        QTRY_COMPARE(test_peerDiconnectedCount, 1);

        test_transportService.disconnectAllPeers();
        QTRY_COMPARE(test_transportLayerMock->m_closeConnectionCount, 1);
        QTRY_COMPARE(test_peerDiconnectedCount, 1);

        test_transportService.stopListening();
        QTRY_COMPARE(test_transportLayerMock->m_stopListeningCount, 1);
    }
};