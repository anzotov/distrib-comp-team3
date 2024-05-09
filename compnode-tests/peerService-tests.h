#pragma once

#include <QObject>
#include <QTest>
#include <functional>
#include <QUuid>

#include "../common/transportServiceBase.h"
#include "../compnode/discoveryServiceBase.h"
#include "../compnode/peerService.h"

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

struct DiscoveryServiceMock final : DiscoveryServiceBase
{
    DiscoveryServiceMock(QObject *parent = nullptr) : DiscoveryServiceBase(parent)
    {
        if (m_contructor)
            m_contructor(this);
    }

    ~DiscoveryServiceMock() override final
    {
        if (m_destructor)
            m_destructor(this);
    }
    void startMulticast(const DiscoveryData &discoveryData) override final
    {
        ++m_startMulticastCount;
        if (m_startMulticast)
            m_startMulticast(this, discoveryData);
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

    void stopMulticast() override final
    {
        ++m_stopMulticastCount;
        if (m_stopMulticast)
            m_stopMulticast(this);
    }

    void clearPeers() override final
    {
        ++m_clearPeersCount;
        if (m_clearPeers)
            m_clearPeers(this);
    }

    QList<DiscoveryData> peers() const override final
    {
        ++m_peersCount;
        return m_peers(this);
    }

    std::function<void(DiscoveryServiceMock *)> m_contructor;
    std::function<void(DiscoveryServiceMock *)> m_destructor;
    std::function<void(DiscoveryServiceMock *, const DiscoveryData &discoveryData)> m_startMulticast;
    int m_startMulticastCount = 0;
    std::function<void(DiscoveryServiceMock *)> m_startListening;
    int m_startListeningCount = 0;
    std::function<void(DiscoveryServiceMock *)> m_stopListening;
    int m_stopListeningCount = 0;
    std::function<void(DiscoveryServiceMock *)> m_stopMulticast;
    int m_stopMulticastCount = 0;
    std::function<void(DiscoveryServiceMock *)> m_clearPeers;
    int m_clearPeersCount = 0;
    std::function<QList<DiscoveryData>(const DiscoveryServiceMock *)> m_peers;
    mutable int m_peersCount = 0;
};

class PeerServiceTestCase final : public QObject
{
    Q_OBJECT

private slots:
    void Test1()
    {
        auto test_transportServiceMock = new TransportServiceMock;
        auto test_discoveryServiceMock = new DiscoveryServiceMock;
        QString test_CompPower = "1000";
        DiscoveryData test_discoveryData(QUuid::createUuid().toString(), {"127.0.0.1:1234"});
        PeerService peerService(test_transportServiceMock, test_discoveryServiceMock, test_discoveryData);
        peerService.start(test_CompPower);
        QTRY_COMPARE(test_transportServiceMock->m_startListeningCount, 1);
        QTRY_COMPARE(test_discoveryServiceMock->m_startListeningCount, 1);
        QTRY_COMPARE(test_discoveryServiceMock->m_startMulticastCount, 1);
    }
};