#pragma once

#include "transportServiceBase.h"

#include "../common/serializerBase.h"
#include "../common/encoderBase.h"

#include <QSet>
#include <QByteArray>
#include <QJsonObject>

class TransportService final : public TransportServiceBase
{
public:
    TransportService(TransportLayerBase *transportLayer,
                     SerializerBase<QByteArray, QJsonObject> *serializer,
                     EncoderBase<QByteArray> *encoder,
                     QObject *parent = nullptr);
    ~TransportService();

    void sendHandshake(const PeerHandlerType &peerHandler, const Handshake &handshake) override final;
    void sendCalcTask(const PeerHandlerType &peerHandler, const CalcTask &task) override final;
    void sendCalcResult(const PeerHandlerType &peerHandler, const CalcResult &result) override final;
    void connectPeer(const QString &peerInfo) override final;
    void disconnectPeer(const PeerHandlerType &peerHandler) override final;
    void disconnectAllPeers() override final;
    QList<PeerHandlerType> peers() const override final;
    void startListening() override final;
    void stopListening() override final;

private:
    void onNewConnection(const PeerHandlerType peerHandler, const QString peerInfo, bool outgoing);
    void onConnectionClosed(const PeerHandlerType peerHandler);
    void onConnectError(const QString peerInfo);
    void onDataReceived(const PeerHandlerType peerHandler, const QByteArray data);

    TransportLayerBase *m_transportLayer = nullptr;
    SerializerBase<QByteArray, QJsonObject> *m_serializer = nullptr;
    EncoderBase<QByteArray> *m_encoder = nullptr;
    QSet<PeerHandlerType> m_peers;
};