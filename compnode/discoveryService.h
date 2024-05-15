#pragma once

#include "discoveryServiceBase.h"
#include "../common/serializerBase.h"

#include <QUdpSocket>
#include <QTimer>
#include <QMap>
#include <optional>

class DiscoveryService : public DiscoveryServiceBase
{
public:
    DiscoveryService(SerializerBase<QByteArray, QJsonObject> *serializer,
                     QHostAddress multicastAddress,
                     quint16 port,
                     std::chrono::milliseconds multicastPeriod,
                     int peersForgetPeriod,
                     QObject *parent = nullptr);
    ~DiscoveryService();

    void startMulticast(const DiscoveryData &discoveryData) override final;
    void startListening() override final;
    void stopListening() override final;
    void stopMulticast() override final;
    void clearPeers() override final;
    QList<DiscoveryData> peers() const override final;

private:
    void onReadyRead();
    void sendData();

    SerializerBase<QByteArray, QJsonObject> *m_serializer = nullptr;
    std::optional<DiscoveryData> m_discoveryData;
    QHostAddress m_multicastAddress;
    QUdpSocket m_socket;
    QTimer m_timer;
    QMap<QString, DiscoveryData> m_peers;
    bool m_listen = false;
    int m_peersForgetPeriod = 0;
    unsigned int m_periodCounter = 0;
};
