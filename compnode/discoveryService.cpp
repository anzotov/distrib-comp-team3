#include "discoveryService.h"

#include "discoveryData.h"
#include "../common/safeHandler.h"

#include <QNetworkDatagram>

DiscoveryService::DiscoveryService(SerializerBase<QByteArray, QJsonObject> *serializer,
                                   QHostAddress multicastAddress,
                                   quint16 port,
                                   std::chrono::milliseconds multicastPeriod,
                                   int peersForgetPeriods,
                                   QObject *parent)
    : DiscoveryServiceBase(parent),
      m_serializer(serializer),
      m_multicastAddress(multicastAddress),
      m_peersForgetPeriod(peersForgetPeriods)
{
    qDebug() << QStringLiteral("DiscoveryService(%1, %2, %3)").arg(multicastAddress.toString()).arg(port).arg(multicastPeriod.count());
    if (!m_serializer)
    {
        throw std::runtime_error("DiscoveryService: not all dependencies were satisfied");
    }
    if (
        !m_socket.bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress))
    {
        throw std::runtime_error("DiscoveryService: unable to bind socket");
    }
    if (
        !m_socket.joinMulticastGroup(multicastAddress))
    {
        throw std::runtime_error("DiscoveryService: unable to join multicast group");
    }
    connect(&m_socket, &QIODevice::readyRead, this, &DiscoveryService::onReadyRead);

    m_timer.setInterval(multicastPeriod);
    connect(&m_timer, &QTimer::timeout, this, &DiscoveryService::sendData);
}

DiscoveryService::~DiscoveryService()
{
    qDebug() << "~DiscoveryService()";
    delete m_serializer;
}

void DiscoveryService::startMulticast(const DiscoveryData &discoveryData)
{
    qDebug() << QStringLiteral("DiscoveryService: startMulticast(%1)").arg(discoveryData.toQString());
    m_discoveryData = discoveryData;
    m_timer.start();
}

void DiscoveryService::startListening()
{
    qDebug() << "DiscoveryService: startListening()";
    m_listen = true;
}

void DiscoveryService::stopListening()
{
    qDebug() << "DiscoveryService: stopListening()";
    m_listen = false;
}

void DiscoveryService::stopMulticast()
{
    qDebug() << "DiscoveryService: stopMulticast()";
    m_timer.stop();
    m_discoveryData.reset();
}

void DiscoveryService::clearPeers()
{
    qDebug() << "DiscoveryService: clearPeers()";
    m_peers.clear();
}

QList<DiscoveryData> DiscoveryService::peers() const
{
    qDebug() << "DiscoveryService: peers()";
    return m_peers.values();
}

void DiscoveryService::onReadyRead()
{
    qDebug() << "DiscoveryService: onReadyRead()";
    QNetworkDatagram datagram = m_socket.receiveDatagram();
    if (!m_listen)
        return;

    if (datagram.isValid() && !datagram.data().isEmpty())
    {
        try
        {
            m_serializer->deserialize<DiscoveryData>(datagram.data(), makeSafeHandler<DiscoveryData>(
                                                                          [this](std::unique_ptr<DiscoveryData> peerData)
                                                                          {
                                                                              if (m_discoveryData && m_discoveryData->uuid != peerData->uuid)
                                                                              {
                                                                                  if (!m_peers.contains(peerData->uuid))
                                                                                  {
                                                                                      m_peers.insert(peerData->uuid, *peerData);
                                                                                      emit peersChanged(m_peers.values());
                                                                                  }
                                                                              }
                                                                          }));
        }
        catch (DeserializationError &e)
        {
            qCritical() << "Data deserialization error:" << e.what();
        }
    }
}

void DiscoveryService::sendData()
{
    qDebug() << "DiscoveryService: sendData()";
    QByteArray data = m_serializer->serialize(*m_discoveryData);
    m_socket.writeDatagram(data, m_multicastAddress, m_socket.localPort());
    if (m_peersForgetPeriod >= 0 && (++m_periodCounter >= m_peersForgetPeriod))
    {
        m_periodCounter = 0;
        clearPeers();
    }
}
