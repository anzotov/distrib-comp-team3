#include "networkMock.h"

#include <QString>
#include <QTimer>
#include <QDebug>
#include <stdexcept>
#include <optional>
#include <chrono>
#include <utility>

struct TransportLayerMock final : TransportLayerBase
{
    TransportLayerMock(NetworkMock *networkMock, const QString &peerInfo, QObject *parent = nullptr)
        : TransportLayerBase(parent), m_networkMock(networkMock), m_peerInfo(peerInfo)
    {
        qDebug() << QStringLiteral("TransportLayerMock(%1)").arg(peerInfo);
        if (!m_networkMock)
            throw std::runtime_error("TransportLayerMock: not all dependencies were satisfied");
    }
    ~TransportLayerMock()
    {
        qDebug() << QStringLiteral("~TransportLayerMock [%1]()").arg(m_peerInfo);
    }
    void sendData(const PeerHandlerType &peerHandler, const QByteArray &data) override final
    {
        qDebug() << QStringLiteral("TransportLayerMock [%1]: sendData(%2)").arg(m_peerInfo).arg(peerHandler);
        if (m_handlerToMockMap.contains(peerHandler) && m_handlerToMockMap[peerHandler] && m_localToRemoteHandlerMap.contains(peerHandler))
        {
            auto &peer = m_handlerToMockMap[peerHandler];
            auto remoteInfo = peer->m_peerInfo;
            auto remoteHandler = m_localToRemoteHandlerMap[peerHandler];
            qDebug() << QStringLiteral("Sending data from %1 (local handler %2) to %3 (remote handler %4)")
                            .arg(m_peerInfo)
                            .arg(peerHandler)
                            .arg(remoteInfo)
                            .arg(remoteHandler);
            QTimer::singleShot(0, peer, [=, this]()
                               { emit peer->dataReceived(remoteHandler, data); });
        }
    }
    void openConnection(const QString &peerInfo) override final
    {
        qDebug() << QStringLiteral("TransportLayerMock [%1]: openConnection(%2)").arg(m_peerInfo).arg(peerInfo);
        if (m_networkMock && m_networkMock->m_peerInfoToTransportMap.contains(peerInfo) && m_networkMock->m_peerInfoToTransportMap[peerInfo])
        {
            auto &peer = m_networkMock->m_peerInfoToTransportMap[peerInfo];
            if (peer->m_listening)
            {
                auto localHandler = QString().setNum(m_nextHandler++);
                auto remoteHandler = QString().setNum((peer->m_nextHandler)++);
                m_handlerToMockMap.insert(localHandler, peer);
                m_localToRemoteHandlerMap.insert(localHandler, remoteHandler);
                peer->m_handlerToMockMap.insert(remoteHandler, this);
                peer->m_localToRemoteHandlerMap.insert(remoteHandler, localHandler);

                qDebug() << QStringLiteral("New connection from %1 (local handler %2) to %3 (remote handler %4)")
                                .arg(m_peerInfo)
                                .arg(localHandler)
                                .arg(peerInfo)
                                .arg(remoteHandler);
                QTimer::singleShot(0, this, [=, this]()
                                   { emit newConnection(localHandler, peerInfo, true); });
                QTimer::singleShot(0, peer, [=, this]()
                                   { emit peer->newConnection(remoteHandler, m_peerInfo, false); });
                return;
            }
        }
        emit connectError(peerInfo);
    }
    void closeConnection(const PeerHandlerType &peerHandler) override final
    {
        qDebug() << QStringLiteral("TransportLayerMock [%1]: closeConnection(%2)").arg(m_peerInfo).arg(peerHandler);
        if (m_handlerToMockMap.contains(peerHandler) && m_handlerToMockMap[peerHandler] && m_localToRemoteHandlerMap.contains(peerHandler))
        {
            auto &peer = m_handlerToMockMap[peerHandler];
            auto remoteInfo = peer->m_peerInfo;
            auto remoteHandler = m_localToRemoteHandlerMap[peerHandler];
            qDebug() << QStringLiteral("Closing connection from %1 (local handler %2) to %3 (remote handler %4)")
                            .arg(m_peerInfo)
                            .arg(peerHandler)
                            .arg(remoteInfo)
                            .arg(remoteHandler);
            QTimer::singleShot(0, this, [=, this]()
                               {
                                   m_handlerToMockMap.remove(peerHandler);
                                   m_localToRemoteHandlerMap.remove(peerHandler); 
                                   emit connectionClosed(peerHandler); });
            QTimer::singleShot(0, peer, [=, this]()
                               {
                                   peer->m_handlerToMockMap.remove(remoteHandler);
                                   peer->m_localToRemoteHandlerMap.remove(remoteHandler); 
                                   emit peer->connectionClosed(remoteHandler); });
        }
    }
    void startListening()
    {
        qDebug() << QStringLiteral("TransportLayerMock [%1]: startListening()").arg(m_peerInfo);
        m_listening = true;
    }
    void stopListening()
    {
        qDebug() << QStringLiteral("TransportLayerMock [%1]: stopListening()").arg(m_peerInfo);
        m_listening = false;
    }

    QPointer<NetworkMock> m_networkMock;
    const QString m_peerInfo;
    bool m_listening = false;
    quint64 m_nextHandler = 0;
    QMap<PeerHandlerType, QPointer<TransportLayerMock>> m_handlerToMockMap;
    QMap<PeerHandlerType, PeerHandlerType> m_localToRemoteHandlerMap;
};

struct DiscoveryServiceMock final : DiscoveryServiceBase
{
    DiscoveryServiceMock(NetworkMock *networkMock, std::chrono::milliseconds multicastPeriod, QObject *parent = nullptr)
        : DiscoveryServiceBase(parent), m_networkMock(networkMock)
    {
        m_timer.setInterval(multicastPeriod);
        m_timer.callOnTimeout(this, &DiscoveryServiceMock::multicast);
    }

    ~DiscoveryServiceMock() override final
    {
    }
    void startMulticast(const DiscoveryData &discoveryData) override final
    {
        m_discoveryData = discoveryData;
        if (m_networkMock)
        {
            m_networkMock->m_uuidToDiscoveryMap.insert(discoveryData.uuid, this);
        }
        m_timer.start();
    }

    void startListening() override final
    {
        m_listening = true;
    }

    void stopListening() override final
    {
        m_listening = false;
    }

    void stopMulticast() override final
    {
        m_timer.stop();
    }

    void clearPeers() override final
    {
        m_peers.clear();
        emit peersChanged(peers());
    }

    QList<DiscoveryData> peers() const override final
    {
        return m_peers.values();
    }

    void multicast()
    {
        if (m_discoveryData && m_networkMock)
        {
            for (auto &peer : m_networkMock->m_uuidToDiscoveryMap.values())
            {
                if (peer != this && peer->m_listening && !peer->m_peers.contains(m_discoveryData->uuid))
                {
                    peer->m_peers.insert(m_discoveryData->uuid, *m_discoveryData);
                    QTimer::singleShot(0, peer, [peer]()
                                       { emit peer->peersChanged(peer->peers()); });
                }
            }
        }
    }

    QPointer<NetworkMock> m_networkMock;
    std::optional<DiscoveryData> m_discoveryData;
    QTimer m_timer;
    bool m_listening = false;
    QMap<QString, DiscoveryData> m_peers;
};

TransportLayerBase *NetworkMock::createTransportLayer(const QString &peerInfo)
{
    auto mock = new TransportLayerMock(this, peerInfo);
    m_peerInfoToTransportMap.insert(peerInfo, mock);
    return mock;
}

DiscoveryServiceBase *NetworkMock::createDiscoveryService(std::chrono::milliseconds multicastPeriod)
{
    return new DiscoveryServiceMock(this, multicastPeriod);
}
