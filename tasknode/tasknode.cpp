#include "tasknode.h"
#include <QTimer>
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
    qDebug() << "TaskNode: start()";
    if (m_state != State::Stopped)
        throw std::logic_error("TaskNode: need to stop before starting again");

    qInfo() << "Task node is starting";
    m_state = State::TaskRequested;
    m_taskProvider->loadNextTask();
}

void TaskNode::onTaskLoadDone(const CalcTask task)
{
    qDebug() << "TaskNode: onTaskLoadDone()";
    if (m_state == State::TaskRequested)
    {
        qInfo() << "Task load done";
        m_state = State::ConnectionRequested;
        m_task = task;
        m_connectAttempt = 0;
        m_transportServiceBase->connectPeer(m_peerInfo);
    }
}

void TaskNode::onTaskLoadError()
{
    qDebug() << "TaskNode: onTaskLoadError()";
    if (m_state == State::TaskRequested)
    {
        qCritical("Task load error");
        stop();
    }
}

void TaskNode::onNoTasksAvailable()
{
    qDebug() << "TaskNode: onNoTasksAvailable()";
    if (m_state == State::TaskRequested)
    {
        qWarning("No tasks available");
        stop();
    }
}

void TaskNode::sendHandshake(const TransportServiceBase::PeerHandlerType &peerHandler)
{
    qDebug() << QStringLiteral("TaskNode: sendHandshake(%1)").arg(peerHandler);
    qInfo() << "Sending handshake to peer";
    m_transportServiceBase->sendHandshake(peerHandler, Handshake(Handshake::PeerType::TaskNode, "", QUuid::createUuid().toString()));
}

void TaskNode::onNewPeer(const TransportServiceBase::PeerHandlerType peerHandler, const QString peerInfo, bool outgoing)
{
    qDebug() << QStringLiteral("TaskNode: onNewPeer(%1, %2, %3)").arg(peerHandler).arg(peerInfo).arg(outgoing);
    if (m_state == State::ConnectionRequested && outgoing && peerInfo == m_peerInfo)
    {
        qInfo() << "Connected to peer";
        m_state = State::HandshakeSent;
        sendHandshake(peerHandler);
    }
    else
    {
        qWarning() << "Unexpected connection";
        m_transportServiceBase->disconnectPeer(peerHandler);
    }
}

void TaskNode::onReceivedHandshake(const TransportServiceBase::PeerHandlerType peerHandler, const Handshake handshake)
{
    qDebug() << QStringLiteral("TaskNode: onReceivedHandshake(%1, %2)").arg(peerHandler).arg(handshake.toQString());
    qInfo() << "Handshake received from peer";
    if (m_state == State::ConnectionRequested)
    {
        m_state = State::HandshakeSent;
        sendHandshake(peerHandler);
    }
    else if (m_state == State::HandshakeSent)
    {
        if (handshake.peerType == Handshake::PeerType::CompNode)
        {
            qInfo() << "Sending task to peer";
            m_state = State::TaskSent;
            m_peerHandler = peerHandler;
            m_transportServiceBase->sendCalcTask(peerHandler, *m_task);
            return;
        }
    }
    else
    {
        qWarning() << "Unexpected handshake";
        m_transportServiceBase->disconnectPeer(peerHandler);
    }
}

void TaskNode::onConnectError(const QString &peerInfo)
{
    qDebug() << QStringLiteral("TaskNode: onConnectError(%1)").arg(peerInfo);
    if (m_state == State::ConnectionRequested)
    {
        qWarning("Connect error");
        if (++m_connectAttempt < m_maxConnectAttempts)
        {
            QTimer::singleShot(m_reconnectTime, this, [this, peerInfo]()
                               { m_transportServiceBase->connectPeer(m_peerInfo); });
        }
        else
        {
            qCritical("Connection attempts are over");
            stop();
        }
    }
}

void TaskNode::onReceivedCalcResult(const TransportServiceBase::PeerHandlerType peerHandler, const CalcResult result)
{
    qDebug() << QStringLiteral("TaskNode: onReceivedCalcResult(%1)").arg(peerHandler);
    if (m_state == State::TaskSent)
    {
        if (result.isMain)
        {
            qInfo() << "Result received";
            m_state = State::FormatRequested;
            m_taskProvider->formatResult(result);
            if (m_peerHandler.has_value())
            {
                m_transportServiceBase->disconnectPeer(*m_peerHandler);
            }
        }
        else
        {
            qCritical("Wrong result type");
            stop();
        }
    }
}
void TaskNode::onResultFormatDone()
{
    qDebug() << "TaskNode: onResultFormatDone()";
    if (m_state == State::FormatRequested)
    {
        qInfo() << "Result formatting done. Task node is stopping";
        m_state = State::Stopped;
        emit stopped(true);
    }
}

void TaskNode::onResultFormatError(CalcResult)
{
    qDebug() << "TaskNode: onResultFormatError()";
    qCritical("Result formatting error");
    stop();
}

void TaskNode::onPeerDiconnected(const TransportServiceBase::PeerHandlerType peerHandler)
{
    qDebug() << QStringLiteral("TaskNode: onPeerDiconnected(%1)").arg(peerHandler);
    if (m_peerHandler.has_value() && *m_peerHandler == peerHandler)
    {
        qInfo() << "Peer disconnected";
        m_peerHandler.reset();
    }
    if (m_state == State::TaskSent)
    {
        qCritical("Connection lost, result will not be received");
        stop();
    }
}

void TaskNode::stop()
{
    qDebug() << "TaskNode: stop()";
    qInfo() << "Task node is stopping";
    m_state = State::Stopped;
    m_transportServiceBase->disconnectAllPeers();
    emit stopped(false);
}
