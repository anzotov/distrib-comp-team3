#pragma once

#include "../common/transportService.h"
#include "../common/jsonSerializer.h"
#include "../common/encoderBase.h"

#include <QObject>
#include <QTest>
#include <functional>

struct EncoderMock final : EncoderBase<QByteArray>
{
    QByteArray encode(const QByteArray &array) const override final
    {
        ++m_encodeCount;
        return m_encode(this, array);
    }
    QByteArray decode(const QByteArray &array) const override final
    {
        ++m_decodeCount;
        return m_decode(this, array);
    }

    std::function<QByteArray(const EncoderMock *, const QByteArray &)> m_encode;
    mutable int m_encodeCount = 0;
    std::function<QByteArray(const EncoderMock *, const QByteArray &)> m_decode;
    mutable int m_decodeCount = 0;
};

struct TransportLayerMock final : TransportLayerBase
{
    TransportLayerMock(QObject *parent = nullptr) : TransportLayerBase(parent)
    {
        if (m_contructor)
            m_contructor(this);
    }
    ~TransportLayerMock()
    {
        if (m_destructor)
            m_destructor(this);
    }
    void sendData(const PeerHandlerType peerHandler, const QByteArray &data) override final
    {
        ++m_sendDataCount;
        if (m_sendData)
            m_sendData(this, peerHandler, data);
    }
    void openConnection(const QString &peerInfo) override final
    {
        ++m_openConnectionCount;
        if (m_openConnection)
            m_openConnection(this, peerInfo);
    }
    void closeConnection(const PeerHandlerType peerHandler) override final
    {
        ++m_closeConnectionCount;
        if (m_closeConnection)
            m_closeConnection(this, peerHandler);
    }
    void startListening()
    {
        ++m_startListeningCount;
        if (m_startListening)
            m_startListening(this);
    }
    void stopListening()
    {
        ++m_stopListeningCount;
        if (m_stopListening)
            m_stopListening(this);
    }

    std::function<void(TransportLayerMock *)> m_contructor;
    std::function<void(TransportLayerMock *)> m_destructor;
    std::function<void(TransportLayerMock *, const PeerHandlerType, const QByteArray &)> m_sendData;
    int m_sendDataCount = 0;
    std::function<void(TransportLayerMock *, const QString &)> m_openConnection;
    int m_openConnectionCount = 0;
    std::function<void(TransportLayerMock *, const PeerHandlerType)> m_closeConnection;
    int m_closeConnectionCount = 0;
    std::function<void(TransportLayerMock *)> m_startListening;
    int m_startListeningCount = 0;
    std::function<void(TransportLayerMock *)> m_stopListening;
    int m_stopListeningCount = 0;
};

class TransportServiceTestCase : public QObject
{
    Q_OBJECT
private slots:
    void Test1()
    {
        QString test_peerInfo = "127.0.0.1:3333";
        TransportLayerBase::PeerHandlerType test_peerHandler = "123";
        Handshake test_handshake(Handshake::PeerType::CompNode, "1000", "{1234-5-6-7}");
        auto test_transportLayerMock = new TransportLayerMock;
        test_transportLayerMock->m_openConnection =
            [&](TransportLayerMock *, const QString &peerInfo)
        {
            QCOMPARE(peerInfo, test_peerInfo);
        };
        test_transportLayerMock->m_sendData = [&](TransportLayerMock *, const TransportLayerMock::PeerHandlerType peerHandler, const QByteArray &data)
        {
            emit test_transportLayerMock->dataReceived(peerHandler, data);
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

        test_transportService.startListening();
        QTRY_COMPARE(test_transportLayerMock->m_startListeningCount, 1);
        test_transportService.connectPeer(test_peerInfo);
        QTRY_COMPARE(test_transportLayerMock->m_openConnectionCount, 1);
        emit test_transportLayerMock->newConnection(test_peerHandler, test_peerInfo, true);
        QTRY_COMPARE(test_newPeerCount, 1);
        test_transportService.sendHandshake(test_peerHandler, test_handshake);
        QTRY_COMPARE(test_transportLayerMock->m_sendDataCount, 1);
        QTRY_COMPARE(test_receivedHandshakeCount, 1);
    }
};