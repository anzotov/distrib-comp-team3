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
}

CompNode::~CompNode()
{
    qDebug() << "~CompNode()";
    delete m_peerService;
    delete m_chunkerService;
}

void CompNode::start()
{
    qInfo() << "CompNode: start";
    if (m_state != State::Stopped)
        throw std::logic_error("CompNode: need to stop before starting again");

    m_state = State::Starting;
    m_chunkerService->start();
}

void CompNode::onChunkerServiceReady(const QString compPower)
{
    if (m_state == State::Starting)
    {
        qInfo() << "CompNode: ChunkerService ready";
        m_state = State::Ready;
        m_peerService->start(compPower);
    }
}

void CompNode::onPeersChanged(const QList<PeerInfo> peers)
{
    qInfo() << "CompNode: peers changed";
    m_chunkerService->updatePeers(peers);
}

void CompNode::onReceivedCalcTask(const PeerHandlerType peerHandler, const CalcTask task)
{
    if (m_state == State::Ready)
    {
        qInfo() << "CompNode: Received CalcTask";
        m_state = State::TaskReceived;
        m_chunkerService->calculateTask(peerHandler, task);
    }
}

void CompNode::onChunkerServiceSendChunkedTask(const PeerHandlerType peerHandler, const CalcTask task)
{
    if (m_state == State::TaskReceived)
    {
        qInfo() << "CompNode: Sending CalcTask to peers";
        m_peerService->sendCalcTask(peerHandler, task);
    }
}

void CompNode::onReceivedCalcResult(const PeerHandlerType peerHandler, const CalcResult result)
{
    if (m_state == State::TaskReceived)
    {
        qInfo() << "CompNode: CalcResult received from peer";
        m_chunkerService->processChunkedResult(peerHandler, result);
    }
}

void CompNode::onChunkerServiceCalcResult(const PeerHandlerType peerHandler, const CalcResult result)
{
    if (m_state == State::TaskReceived)
    {
        qInfo() << "CompNode: CalcResult received from ChunkerService";
        m_state = State::Ready;
        m_peerService->sendCalcResult(peerHandler, result);
    }
}

void CompNode::onChunkerServiceCalcError()
{
    if (m_state == State::TaskReceived)
    {
        qInfo() << "CompNode: Error received from ChunkerService";
        m_state = State::Starting;
        m_chunkerService->stop();
        m_peerService->stop();
        m_chunkerService->start();
    }
}

void CompNode::stop()
{
    qInfo() << "CompNode: stop";
    m_state = State::Stopped;
    m_chunkerService->stop();
    m_peerService->stop();
    emit stopped();
}
