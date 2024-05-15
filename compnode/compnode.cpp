#include "compnode.h"
#include <stdexcept>

CompNode::CompNode(PeerServiceBase *peerService, ChunkerServiceBase *chunkerService, QObject *parent)
    : QObject(parent),
      m_peerService(peerService),
      m_chunkerService(chunkerService)
{
    qDebug() << "CompNode()";
    if (!m_peerService || !m_chunkerService)
    {
        throw std::runtime_error("CompNode: not all dependencies were satisfied");
    }
    QObject::connect(m_chunkerService, &ChunkerServiceBase::ready, this, &CompNode::onChunkerServiceReady);
    QObject::connect(m_chunkerService, &ChunkerServiceBase::calcResult, this, &CompNode::onChunkerServiceCalcResult);
    QObject::connect(m_chunkerService, &ChunkerServiceBase::calcError, this, &CompNode::onChunkerServiceCalcError);
    QObject::connect(m_chunkerService, &ChunkerServiceBase::sendChunkedTask, this, &CompNode::onChunkerServiceSendChunkedTask);
    QObject::connect(m_peerService, &PeerServiceBase::receivedCalcTask, this, &CompNode::onReceivedCalcTask);
    QObject::connect(m_peerService, &PeerServiceBase::receivedCalcResult, this, &CompNode::onReceivedCalcResult);
    QObject::connect(m_peerService, &PeerServiceBase::peersChanged, this, &CompNode::onPeersChanged);
    QObject::connect(m_peerService, &PeerServiceBase::taskNodeDisconnected, this, &CompNode::onTaskNodeDisconnected);
}

CompNode::~CompNode()
{
    qDebug() << "~CompNode()";
    delete m_peerService;
    delete m_chunkerService;
}

void CompNode::start()
{
    qDebug() << "CompNode: start()";
    if (m_state != State::Stopped)
        throw std::logic_error("CompNode: need to stop before starting again");

    qInfo() << "Node is starting";
    m_state = State::Starting;
    m_chunkerService->start();
}

void CompNode::onChunkerServiceReady(const QString compPower)
{
    qDebug() << "CompNode: onChunkerServiceReady()";
    if (m_state == State::Starting)
    {
        qInfo() << "Node is ready";
        m_state = State::Ready;
        m_peerService->start(compPower);
    }
}

void CompNode::onPeersChanged(const QList<PeerInfo> peers)
{
    qDebug() << "CompNode: onPeersChanged()";
    qInfo() << "Peer list changed";
    m_chunkerService->updatePeers(peers);
}

void CompNode::onTaskNodeDisconnected(const PeerHandlerType peerHandler)
{
    qDebug() << QStringLiteral("CompNode: onTaskNodeDisconnected(%1)").arg(peerHandler);
    if (m_state == State::TaskReceived && peerHandler == m_taskSourceHandler)
    {
        qCritical() << "Task node disconnected, calculation will not be completed";
        restartNode();
    }
}

void CompNode::onReceivedCalcTask(const PeerHandlerType peerHandler, const CalcTask task)
{
    qDebug() << QStringLiteral("CompNode: onReceivedCalcTask(%1)").arg(peerHandler);
    if (m_state == State::Ready)
    {
        qInfo() << "Task is received";
        m_state = State::TaskReceived;
        m_taskSourceHandler = peerHandler;
        m_chunkerService->calculateTask(peerHandler, task);
    }
}

void CompNode::onChunkerServiceSendChunkedTask(const PeerHandlerType peerHandler, const CalcTask task)
{
    qDebug() << QStringLiteral("CompNode: onChunkerServiceSendChunkedTask(%1)").arg(peerHandler);
    if (m_state == State::TaskReceived)
    {
        qInfo() << "Sending chunked task to peer";
        m_peerService->sendCalcTask(peerHandler, task);
    }
}

void CompNode::onReceivedCalcResult(const PeerHandlerType peerHandler, const CalcResult result)
{
    qDebug() << QStringLiteral("CompNode: onReceivedCalcResult(%1)").arg(peerHandler);
    if (m_state == State::TaskReceived)
    {
        qInfo() << "Task result is received from peer";
        m_chunkerService->processChunkedResult(peerHandler, result);
    }
}

void CompNode::onChunkerServiceCalcResult(const PeerHandlerType peerHandler, const CalcResult result)
{
    qDebug() << QStringLiteral("CompNode: onChunkerServiceCalcResult(%1)").arg(peerHandler);
    if (m_state == State::TaskReceived)
    {
        qInfo() << "Sending task result to peer";
        m_state = State::Ready;
        m_peerService->sendCalcResult(peerHandler, result);
    }
}

void CompNode::onChunkerServiceCalcError()
{
    qDebug() << "CompNode: onChunkerServiceCalcError()";
    if (m_state == State::TaskReceived)
    {
        qCritical() << "Task calculation error";
        restartNode();
    }
}

void CompNode::stop()
{
    qDebug() << "CompNode: stop()";
    qInfo() << "Node is stopping";
    m_state = State::Stopped;
    m_chunkerService->stop();
    m_peerService->stop();
    emit stopped();
}

void CompNode::restartNode()
{
    qDebug() << "CompNode: restartNode()";
    qCritical() << "Restarting node";
    m_state = State::Starting;
    m_chunkerService->stop();
    m_peerService->stop();
    m_chunkerService->start();
}
