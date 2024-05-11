#include "transportService.h"

TransportService::TransportService(TransportLayerBase *transportLayer,
                                   SerializerBase<QByteArray, QJsonObject> *serializer,
                                   EncoderBase<QByteArray> *encoder,
                                   QObject *parent)
    : TransportServiceBase(parent),
      m_transportLayer(transportLayer),
      m_serializer(serializer),
      m_encoder(encoder)
{
    qDebug() << "TransportService()";
    if (!m_transportLayer || !m_serializer || !m_encoder)
    {
        throw std::runtime_error("TransportService: not all dependencies were satisfied");
    }
    QObject::connect(m_transportLayer, &TransportLayerBase::newConnection, this, &TransportService::onNewConnection);
    QObject::connect(m_transportLayer, &TransportLayerBase::connectionClosed, this, &TransportService::onConnectionClosed);
    QObject::connect(m_transportLayer, &TransportLayerBase::connectError, this, &TransportService::onConnectError);
    QObject::connect(m_transportLayer, &TransportLayerBase::dataReceived, this, &TransportService::onDataReceived);
}

TransportService::~TransportService()
{
    qDebug() << "~TransportService()";
    delete m_transportLayer;
    delete m_serializer;
    delete m_encoder;
}

void TransportService::sendHandshake(const PeerHandlerType &peerHandler, const Handshake &handshake)
{
    qDebug() << QStringLiteral("TransportService: sendHandshake(%1, %2)").arg(peerHandler).arg(handshake.toQString());
    m_transportLayer->sendData(peerHandler, m_encoder->encode(m_serializer->serialize(handshake)));
}

void TransportService::sendCalcTask(const PeerHandlerType &peerHandler, const CalcTask &task)
{
    qDebug() << QStringLiteral("TransportService: sendCalcTask(%1)").arg(peerHandler);
    m_transportLayer->sendData(peerHandler, m_encoder->encode(m_serializer->serialize(task)));
}

void TransportService::sendCalcResult(const PeerHandlerType &peerHandler, const CalcResult &result)
{
    qDebug() << QStringLiteral("TransportService: sendCalcResult(%1)").arg(peerHandler);
    m_transportLayer->sendData(peerHandler, m_encoder->encode(m_serializer->serialize(result)));
}

void TransportService::connectPeer(const QString &peerInfo)
{
    qDebug() << QStringLiteral("TransportService: connectPeer(%1)").arg(peerInfo);
    m_transportLayer->openConnection(peerInfo);
}

void TransportService::disconnectPeer(const PeerHandlerType &peerHandler)
{
    qDebug() << QStringLiteral("TransportService: disconnectPeer(%1)").arg(peerHandler);
    m_transportLayer->closeConnection(peerHandler);
}

void TransportService::disconnectAllPeers()
{
    qDebug() << "TransportService: disconnectAllPeers()";
    for (const auto &peer : m_peers)
    {
        m_transportLayer->closeConnection(peer);
    }
}

QList<TransportServiceBase::PeerHandlerType> TransportService::peers() const
{
    return m_peers.values();
}

void TransportService::startListening()
{
    qDebug() << "TransportService: startListening";
    m_transportLayer->startListening();
}

void TransportService::stopListening()
{
    qDebug() << "TransportService: stopListening";
    m_transportLayer->stopListening();
}

void TransportService::onNewConnection(const PeerHandlerType peerHandler, const QString peerInfo, bool outgoing)
{
    qDebug() << QStringLiteral("TransportService: onNewConnection(%1, %2, %3)").arg(peerHandler).arg(peerInfo).arg(outgoing);
    m_peers.insert(peerHandler);
    emit newPeer(peerHandler, peerInfo, outgoing);
}

void TransportService::onConnectionClosed(const PeerHandlerType peerHandler)
{
    qDebug() << QStringLiteral("TransportService: onConnectionClosed(%1)").arg(peerHandler);
    m_peers.remove(peerHandler);
    emit peerDiconnected(peerHandler);
}

void TransportService::onConnectError(const QString peerInfo)
{
    qDebug() << QStringLiteral("TransportService: onConnectError(%1)").arg(peerInfo);
    emit connectError(peerInfo);
}

void TransportService::onDataReceived(const PeerHandlerType peerHandler, const QByteArray data)
{
    qDebug() << QStringLiteral("TransportService: onDataReceived(%1)").arg(peerHandler);
    try
    {
        m_serializer->deserialize<Handshake, CalcTask, CalcResult>(m_encoder->decode(data), [&](Handshake *object)
                                                                   {
                                                                        emit receivedHandshake(peerHandler, *object);
                                                                        delete object; }, [&](CalcTask *object)
                                                                   {
                                                                       emit receivedCalcTask(peerHandler, *object);
                                                                       delete object; }, [&](CalcResult *object)
                                                                   {
                                                                       emit receivedCalcResult(peerHandler, *object);
                                                                       delete object; });
    }
    catch (const std::logic_error &e)
    {
        qCritical() << e.what();
    }
}
