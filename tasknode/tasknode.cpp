#include "tasknode.h"
#include <QTimer>
#include <QtGlobal>
#include <QUuid>
#include <stdexcept>

TaskNode::TaskNode(TransportServiceBase *transportServiceBase,
                   TaskProvider *taskProvider,
                   const QString &peerInfo,
                   int maxConnectAttempts,
                   int reconnectTime,
                   QObject *parent)
    : QObject(parent),
      m_transportServiceBase(transportServiceBase),
      m_taskProvider(taskProvider),
      m_peerInfo(peerInfo),
      m_maxConnectAttempts(maxConnectAttempts),
      m_reconnectTime(reconnectTime)
{
    qDebug() << QStringLiteral("TaskNode(%1, %2, %3)").arg(peerInfo).arg(maxConnectAttempts).arg(reconnectTime);
    if (!m_transportServiceBase || !m_taskProvider)
    {
        throw std::runtime_error("TaskNode: not all dependencies were satisfied");
    }
    QObject::connect(m_transportServiceBase, &TransportServiceBase::receivedCalcResult,
                     this, &TaskNode::onReceivedCalcResult);
    QObject::connect(m_transportServiceBase, &TransportServiceBase::receivedHandshake, this, &TaskNode::onReceivedHandshake);
    QObject::connect(m_transportServiceBase, &TransportServiceBase::newPeer, this, &TaskNode::onNewPeer);
    QObject::connect(m_transportServiceBase, &TransportServiceBase::connectError, this, &TaskNode::onConnectError);
    QObject::connect(m_transportServiceBase, &TransportServiceBase::peerDiconnected, this, &TaskNode::onPeerDiconnected);
    QObject::connect(m_taskProvider, &TaskProvider::taskLoadDone, this, &TaskNode::onTaskLoadDone);
    QObject::connect(m_taskProvider, &TaskProvider::taskLoadError, this, &TaskNode::onTaskLoadError);
    QObject::connect(m_taskProvider, &TaskProvider::noTasksAvailable, this, &TaskNode::onNoTasksAvailable);
    QObject::connect(m_taskProvider, &TaskProvider::resultFormatDone, this, &TaskNode::onResultFormatDone);
    QObject::connect(m_taskProvider, &TaskProvider::resultFormatError, this, &TaskNode::onResultFormatError);
}

TaskNode::~TaskNode()
{
    qDebug() << "~TaskNode()";
    delete m_transportServiceBase;
    delete m_taskProvider;
}

void TaskNode::start()
{
    if (m_state != State::Stopped)
        throw std::logic_error("TaskNode: need to stop before starting again");

    qInfo() << "TaskNode: start";
    m_state = State::Started;
    QTimer::singleShot(0, [this]()
                       { 
                            if(m_state == State::Started)
                            {
                                m_state = State::TaskRequested;
                                m_taskProvider->loadNextTask(); 
                            } });
}

void TaskNode::onTaskLoadDone(const CalcTask task)
{
    if (m_state == State::TaskRequested)
    {
        qInfo() << "TaskNode: task loaded";
        m_state = State::ConnectionRequested;
        m_task = task;
        m_connectAttempt = 0;
        m_transportServiceBase->connectPeer(m_peerInfo);
    }
}

void TaskNode::onTaskLoadError()
{
    if (m_state == State::TaskRequested)
    {
        qCritical("TaskNode: task load error!");
        stop();
    }
}

void TaskNode::onNoTasksAvailable()
{
    if (m_state == State::TaskRequested)
    {
        qWarning("TaskNode: no tasks available");
        stop();
    }
}

void TaskNode::sendHandshake(const TransportServiceBase::PeerHandlerType &peerHandler)
{
    qInfo() << "TaskNode: sending handshake";
    m_transportServiceBase->sendHandshake(peerHandler, Handshake(Handshake::PeerType::TaskNode, "", QUuid::createUuid().toString()));
}

void TaskNode::onNewPeer(const TransportServiceBase::PeerHandlerType peerHandler, const QString peerInfo, bool outgoing)
{
    if (m_state == State::ConnectionRequested && outgoing && peerInfo == m_peerInfo)
    {
        qInfo() << "TaskNode: connected to peer";
        m_state = State::HandshakeSent;
        sendHandshake(peerHandler);
    }
    else
    {
        m_transportServiceBase->disconnectPeer(peerHandler);
    }
}

void TaskNode::onReceivedHandshake(const TransportServiceBase::PeerHandlerType peerHandler, const Handshake handshake)
{
    if (m_state == State::ConnectionRequested)
    {
        m_state = State::HandshakeSent;
        sendHandshake(peerHandler);
    }
    if (m_state == State::HandshakeSent)
    {
        qInfo() << "TaskNode: received handshake";
        if (handshake.peerType == Handshake::PeerType::CompNode)
        {
            qInfo() << "TaskNode: sending task";
            m_state = State::TaskSent;
            m_peerHandler = peerHandler;
            m_transportServiceBase->sendCalcTask(peerHandler, *m_task);
            return;
        }
    }
    qCritical() << "Unexpected handshake";
    stop();
}

void TaskNode::onConnectError(const QString &peerInfo)
{
    if (m_state == State::ConnectionRequested)
    {
        qCritical("TaskNode: connect error");
        if (++m_connectAttempt < m_maxConnectAttempts)
        {
            QTimer::singleShot(m_reconnectTime, [this, peerInfo]()
                               { m_transportServiceBase->connectPeer(m_peerInfo); });
        }
        else
        {
            stop();
        }
    }
}

void TaskNode::onReceivedCalcResult(const TransportServiceBase::PeerHandlerType peerHandler, const CalcResult result)
{
    (void)peerHandler;
    if (m_state == State::TaskSent)
    {
        if (result.isMain)
        {
            qInfo() << "TaskNode: result received";
            m_state = State::FormatRequested;
            m_taskProvider->formatResult(result);
            if (m_peerHandler.has_value())
            {
                m_transportServiceBase->disconnectPeer(*m_peerHandler);
            }
        }
        else
        {
            qCritical("TaskNode: wrong result type");
            stop();
        }
    }
}
void TaskNode::onResultFormatDone()
{
    if (m_state == State::FormatRequested)
    {
        qInfo() << "TaskNode: result formatted";
        m_state = State::Stopped;
        emit stopped(true);
    }
}

void TaskNode::onResultFormatError(CalcResult result)
{
    (void)result;
    qCritical("TaskNode: result output error");
    stop();
}

void TaskNode::onPeerDiconnected(const TransportServiceBase::PeerHandlerType peerHandler)
{
    if (m_peerHandler.has_value() && *m_peerHandler == peerHandler)
    {
        qInfo() << "TaskNode: peer disconnected";
        m_peerHandler.reset();
    }
    if (m_state == State::TaskSent)
    {
        qCritical("TaskNode: connection lost, result will not be received");
        stop();
    }
}

void TaskNode::stop()
{
    qInfo() << "TaskNode: stopped";
    m_state = State::Stopped;
    m_transportServiceBase->disconnectAllPeers();
    emit stopped(false);
}
