#include "transportLayer.h"

#include <QStringLiteral>
#include <QDataStream>

TransportLayer::TransportLayer(const quint16 &tcpPortValue,
                               QObject *parent)
    : TransportLayerBase(parent),
      m_server(new QTcpServer(this)),
      m_maxPeerHandler(0),
      m_port(tcpPortValue)
{
    m_socketMap.clear();
    this->startListening();
}

TransportLayer::~TransportLayer()
{
    for (auto key : m_socketMap.keys())
    {
        m_socketMap.take(key)->disconnectFromHost();
    }
}

quint16 TransportLayer::getPort() const
{
    return m_server->serverPort();
}

void TransportLayer::openConnection(const QString &peerInfo)
{
    int delimiterPosition = peerInfo.indexOf(':');
    if (delimiterPosition == -1)
    {
        qDebug() << "Connection can't be established: wrong peer info format (transport layer)";
        return;
    }
    QHostAddress peerIPv4 = QHostAddress(peerInfo.left(delimiterPosition - 1));
    quint16 peerPort = peerInfo.right(delimiterPosition + 1).toUInt();

    QPointer<QTcpSocket> socket = new QTcpSocket(this);
    socket->connectToHost(peerIPv4, peerPort);
    if (!socket->isValid())
    {
        qDebug() << "Connection refused (transport layer)";
        return;
    }
    ++m_maxPeerHandler;
    m_socketMap.insert(PeerHandlerType::number(m_maxPeerHandler), socket);
    QObject::connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    qDebug() << QStringLiteral("Connected to host %1 (transport layer)").arg(peerInfo);
}

void TransportLayer::closeConnection(const PeerHandlerType &peerHandler)
{
    QTcpSocket *socket = m_socketMap.take(peerHandler);
    qDebug() << QStringLiteral("Disconnected from host %1:%2 (transport layer)")
                .arg(socket->peerAddress().toString())
                .arg(socket->peerPort());
    socket->disconnectFromHost();
    m_socketMap.remove(peerHandler);
}

void TransportLayer::sendData(const PeerHandlerType &peerHandler, const QByteArray &data)
{
    m_socketMap.take(peerHandler)->write(data);
}

void TransportLayer::startListening()
{
    if (this->m_server->listen(QHostAddress::LocalHost, this->m_port))
    {
        qDebug() << QStringLiteral("Server started listening port %1 on IP %2 (transport layer)")
                    .arg(m_port)
                    .arg(m_server->serverAddress().toString());
    }
    else
    {
        qDebug() << "Server error (transport layer)";
    }
}

void TransportLayer::stopListening()
{
    this->m_server->close();
    qDebug() << QStringLiteral("Server stoped listening on IP %1")
                .arg(this->m_server->serverAddress().toString());
}
