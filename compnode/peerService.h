#pragma once

#include "peerServiceBase.h"
#include "discoveryServiceBase.h"

#include <optional>

class PeerService final : public PeerServiceBase
{
public:
    PeerService(TransportServiceBase *transportService, DiscoveryServiceBase *discoveryService, DiscoveryData discoveryData, QObject *parent = nullptr);
    ~PeerService();

    void start(const QString &compPower) override final;
    void stop() override final;
    void sendCalcTask(const PeerHandlerType &peerHandler, const CalcTask &task) override final;
    void sendCalcResult(const PeerHandlerType &peerHandler, const CalcResult &result) override final;
    QList<PeerInfo> peers() const override final;

private:
    struct PeerRecord
    {
        PeerRecord() = default;
        PeerRecord(const Handshake &handshake) : handshake(handshake) {}
        Handshake handshake;
        QSet<TransportServiceBase::PeerHandlerType> transportHandlers;
    };

    void onReceivedHandshake(const TransportServiceBase::PeerHandlerType peerHandler, const Handshake handshake);
    void onReceivedCalcTask(const TransportServiceBase::PeerHandlerType peerHandler, const CalcTask task);
    void onReceivedCalcResult(const TransportServiceBase::PeerHandlerType peerHandler, const CalcResult result);
    void onNewPeer(const TransportServiceBase::PeerHandlerType peerHandler, const QString peerInfo, bool outgoing);
    void onPeerDiconnected(const TransportServiceBase::PeerHandlerType peerHandler);
    void onConnectError(const QString &peerInfo);
    void removeConnectRequest(const QString &peerInfo);
    void onPeersChanged(const QList<DiscoveryData> peers);

    TransportServiceBase *m_transportService;
    DiscoveryServiceBase *m_discoveryService;
    QString m_compPower;
    DiscoveryData m_discoveryData;
    QMap<QString, PeerRecord> m_uuidToRecordMap;
    QMap<TransportServiceBase::PeerHandlerType, QString> m_transportHandlerToUuidMap;
    QSet<QString> m_connectRequests;
    QMap<TransportServiceBase::PeerHandlerType, QString> m_outboundConnections;
};