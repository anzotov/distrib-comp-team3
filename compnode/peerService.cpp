#include "peerService.h"

#include <QUuid>

PeerService::PeerService(TransportServiceBase *transportService, DiscoveryServiceBase *discoveryService, DiscoveryData discoveryData, QObject *parent)
    : PeerServiceBase(parent),
      m_transportService(transportService),
      m_discoveryService(discoveryService),
      m_discoveryData(discoveryData)
{
    qDebug() << QStringLiteral("PeerService(%1)").arg(discoveryData.toQString());
    if (!m_transportService || !m_discoveryService)
    {
        throw std::runtime_error("PeerService: not all dependencies were satisfied");
    }
    QObject::connect(m_transportService, &TransportServiceBase::receivedHandshake, this, &PeerService::onReceivedHandshake);
    QObject::connect(m_transportService, &TransportServiceBase::receivedCalcTask, this, &PeerService::onReceivedCalcTask);
    QObject::connect(m_transportService, &TransportServiceBase::receivedCalcResult, this, &PeerService::onReceivedCalcResult);
    QObject::connect(m_transportService, &TransportServiceBase::newPeer, this, &PeerService::onNewPeer);
    QObject::connect(m_transportService, &TransportServiceBase::peerDiconnected, this, &PeerService::onPeerDiconnected);
    QObject::connect(m_transportService, &TransportServiceBase::connectError, this, &PeerService::onConnectError);
    QObject::connect(m_discoveryService, &DiscoveryServiceBase::peersChanged, this, &PeerService::onPeersChanged);
}

PeerService::~PeerService()
{
    qDebug() << "~PeerService()";
    delete m_transportService;
    delete m_discoveryService;
}

void PeerService::start(const QString &compPower)
{
    qDebug() << QStringLiteral("PeerService: start(%1)").arg(compPower);
    m_compPower = compPower;
    m_transportService->startListening();
    m_discoveryService->startListening();
    m_discoveryService->startMulticast(m_discoveryData);
}

void PeerService::stop()
{
    qDebug() << "PeerService: stop";
    m_transportService->stopListening();
    m_discoveryService->stopListening();
    m_discoveryService->stopMulticast();
    m_discoveryService->clearPeers();
    m_transportService->disconnectAllPeers();
}

void PeerService::sendCalcTask(const PeerHandlerType &peerHandler, const CalcTask &task)
{
    qDebug() << QStringLiteral("PeerService: sendCalcTask(%1)").arg(peerHandler);
    auto recordIter = m_uuidToRecordMap.find(peerHandler);
    if (recordIter == m_uuidToRecordMap.end())
    {
        qCritical() << QStringLiteral("PeerService: unknown uuid %1").arg(peerHandler);
        return;
    }
    if (recordIter->transportHandlers.isEmpty())
    {
        qCritical() << QStringLiteral("PeerService: no connections to %1").arg(peerHandler);
        return;
    }
    m_transportService->sendCalcTask(recordIter->transportHandlers.first(), task);
}

void PeerService::sendCalcResult(const PeerHandlerType &peerHandler, const CalcResult &result)
{
    qDebug() << QStringLiteral("PeerService: sendCalcResult(%1)").arg(peerHandler);
    auto recordIter = m_uuidToRecordMap.find(peerHandler);
    if (recordIter == m_uuidToRecordMap.end())
    {
        qCritical() << QStringLiteral("PeerService: unknown uuid %1").arg(peerHandler);
        return;
    }
    if (recordIter->transportHandlers.isEmpty())
    {
        qCritical() << QStringLiteral("PeerService: no connections to %1").arg(peerHandler);
        return;
    }
    m_transportService->sendCalcResult(recordIter->transportHandlers.first(), result);
}

QList<PeerInfo> PeerService::peers() const
{
    QList<PeerInfo> list;
    for (const auto &peer : m_uuidToRecordMap)
    {
        if (peer.handshake.peerType == Handshake::PeerType::CompNode)
        {
            list.append(PeerInfo{peer.transportHandlers.first(), peer.handshake.compPower, ""});
        }
    }
    return list;
}

void PeerService::onReceivedHandshake(const TransportServiceBase::PeerHandlerType peerHandler, const Handshake handshake)
{
    qDebug() << QStringLiteral("PeerService: onReceivedHandshake(%1, %2)").arg(peerHandler).arg(handshake.toQString());
    if (!m_uuidToRecordMap.contains(handshake.uuid))
    {
        m_uuidToRecordMap.insert(handshake.uuid, PeerRecord(handshake));
    }

    auto &record = m_uuidToRecordMap[handshake.uuid];
    if (m_discoveryData.uuid > handshake.uuid && !record.transportHandlers.isEmpty())
    {
        m_transportService->disconnectPeer(peerHandler);
        return;
    }

    record.transportHandlers.append(peerHandler);
    m_transportHandlerToUuidMap.insert(peerHandler, handshake.uuid);
}

void PeerService::onReceivedCalcTask(const TransportServiceBase::PeerHandlerType peerHandler, const CalcTask task)
{
    qDebug() << QStringLiteral("PeerService: onReceivedCalcTask(%1)").arg(peerHandler);
    if (!m_transportHandlerToUuidMap.contains(peerHandler))
    {
        qCritical() << QStringLiteral("PeerService: unknown peer %1").arg(peerHandler);
        return;
    }
    emit receivedCalcTask(m_transportHandlerToUuidMap[peerHandler], task);
}

void PeerService::onReceivedCalcResult(const TransportServiceBase::PeerHandlerType peerHandler, const CalcResult result)
{
    qDebug() << QStringLiteral("PeerService: onReceivedCalcResult(%1)").arg(peerHandler);
    if (!m_transportHandlerToUuidMap.contains(peerHandler))
    {
        qCritical() << QStringLiteral("PeerService: unknown peer %1").arg(peerHandler);
        return;
    }
    emit receivedCalcResult(m_transportHandlerToUuidMap[peerHandler], result);
}

void PeerService::onNewPeer(const TransportServiceBase::PeerHandlerType peerHandler, const QString peerInfo, bool outgoing)
{
    qDebug() << QStringLiteral("PeerService: onNewPeer(%1, %2, %3)").arg(peerHandler).arg(peerInfo).arg(outgoing);
    if (outgoing)
    {
        removeConnectRequest(peerInfo);
    }
    m_transportService->sendHandshake(peerHandler, Handshake(Handshake::PeerType::CompNode, m_compPower, m_discoveryData.uuid));
}

void PeerService::onPeerDiconnected(const TransportServiceBase::PeerHandlerType peerHandler)
{
    qDebug() << QStringLiteral("PeerService: onPeerDiconnected(%1)").arg(peerHandler);
    if (!m_transportHandlerToUuidMap.contains(peerHandler))
        return;

    auto uuid = m_transportHandlerToUuidMap[peerHandler];
    auto recordIter = m_uuidToRecordMap.find(uuid);
    if (recordIter == m_uuidToRecordMap.end())
    {
        qCritical() << "PeerService: peer not found";
        return;
    }

    m_transportHandlerToUuidMap.remove(peerHandler);

    auto count = recordIter->transportHandlers.removeAll(peerHandler);
    if (count != 1)
    {
        qWarning() << QStringLiteral("PeerService: wrong number of peerHandlers %1 : %2").arg(peerHandler).arg(count);
    }

    if (recordIter->transportHandlers.isEmpty())
    {
        auto peerType = recordIter->handshake.peerType;
        m_uuidToRecordMap.erase(recordIter);
        if (peerType == Handshake::PeerType::CompNode)
        {
            emit peersChanged(peers());
        }
        else
        {
            emit taskNodeDisconnected(uuid);
        }
    }
}

void PeerService::onConnectError(const QString &peerInfo)
{
    qDebug() << QStringLiteral("PeerService: onConnectError(%1)").arg(peerInfo);
    removeConnectRequest(peerInfo);
}

void PeerService::removeConnectRequest(const QString &peerInfo)
{
    qDebug() << QStringLiteral("PeerService: removeConnectRequest(%1)").arg(peerInfo);
    auto result = m_connectRequests.remove(peerInfo);
    if (!result)
    {
        qWarning() << QStringLiteral("PeerService: uncounted connect request to %1").arg(peerInfo);
    }
}

void PeerService::onPeersChanged(const QList<DiscoveryData> peers)
{
    qDebug() << "PeerService: onPeersChanged()";
    for (const auto &peer : peers)
    {
        if (m_uuidToRecordMap.contains(peer.uuid))
            continue;

        for (const auto &info : peer.connectInfo)
        {
            if (m_connectRequests.contains(info))
                continue;

            m_connectRequests.insert(info);
            m_transportService->connectPeer(info);
        }
    }
}
