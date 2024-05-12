#pragma once

#include "../common/transportLayerBase.h"
#include "../compnode/discoveryServiceBase.h"

#include <QMap>
#include <QPointer>

struct TransportLayerMock;
struct DiscoveryServiceMock;

class NetworkMock final : public QObject
{
    Q_OBJECT

public:
    TransportLayerBase *createTransportLayer(const QString &peerInfo);
    DiscoveryServiceBase *createDiscoveryService(std::chrono::milliseconds multicastPeriod);

    QMap<QString, QPointer<TransportLayerMock>> m_peerInfoToTransportMap;
    QMap<QString, QPointer<DiscoveryServiceMock>> m_uuidToDiscoveryMap;
};