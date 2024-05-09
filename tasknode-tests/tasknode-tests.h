#pragma once

#include "../tasknode/tasknode.h"

#include <QObject>
#include <QTest>

struct TransportServiceMock : TransportServiceBase
{
public:
    void sendHandshake(const PeerHandlerType &peerHandler, const Handshake &handshake) override final
    {
        if (handshake.peerType != Handshake::PeerType::TaskNode)
        {
            qCritical() << "Invalid handshake";
            m_testFail = true;
            return;
        }
        emit receivedHandshake(peerHandler, Handshake(Handshake::PeerType::CompNode, QStringLiteral("1000")));
    }

    void sendCalcTask(const PeerHandlerType &peerHandler, const CalcTask &task) override final
    {
        if (m_peerList.contains(peerHandler))
        {
            m_tasks.append(task);
            emit receivedCalcTask(peerHandler, task);
            return;
        }
        qCritical() << "Invalid handler";
        m_testFail = true;
    }
    void sendCalcResult(const PeerHandlerType &peerHandler, const CalcResult &result) override final
    {
        if (m_peerList.contains(peerHandler))
        {
            m_results.append(result);
            emit receivedCalcResult(peerHandler, result);
            return;
        }
        qCritical() << "Invalid handler";
        m_testFail = true;
    }
    void connectPeer(const QString &peerInfo) override final
    {
        ++m_connectAttempts;
        if (m_connectError)
        {
            emit connectError(peerInfo);
            return;
        }
        m_peerList.append(QString::number(m_handler++));
        emit newPeer(m_peerList.back(), peerInfo, true);
    }
    void disconnectPeer(const PeerHandlerType &peerHandler) override final
    {
        if (m_peerList.contains(peerHandler))
        {
            m_peerList.removeAll(peerHandler);
            emit peerDiconnected(peerHandler);
            return;
        }
        qCritical() << "Invalid handler";
        m_testFail = true;
    }
    void disconnectAllPeers() override final
    {
        while (!m_peerList.empty())
        {
            auto peerHandler = m_peerList.back();
            m_peerList.pop_back();
            emit peerDiconnected(peerHandler);
        }
    }
    QList<PeerHandlerType> peers() const override final
    {
        return m_peerList;
    }

    QList<CalcTask> m_tasks;
    QList<CalcResult> m_results;
    QList<PeerHandlerType> m_peerList;
    quint64 m_handler;
    bool m_connectError = false;
    bool m_testFail = false;
    int m_connectAttempts = 0;
};

struct TaskProviderMock : TaskProvider
{
    void loadNextTask() override final
    {
        if (m_loadError)
        {
            emit taskLoadError();
            return;
        }
        if (m_noTasks)
        {
            emit noTasksAvailable();
            return;
        }
        auto task = m_tasks.at(m_task_index++);
        emit taskLoadDone(task);
    }
    void formatResult(const CalcResult &result) override final
    {
        if (m_formatError)
        {
            emit resultFormatError(result);
            return;
        }
        m_results.append(result);
        emit resultFormatDone();
    }
    QList<CalcTask> m_tasks;
    QList<CalcResult> m_results;
    size_t m_task_index = 0;
    bool m_testFail = false;
    bool m_loadError = false;
    bool m_noTasks = false;
    bool m_formatError = false;
};

class TaskNodeTests : public QObject
{
    Q_OBJECT

private slots:
    void PositiveTest()
    {
        const auto testTask = CalcTask("sin(x)", {"0", "1"}, true);
        const auto testResult = CalcResult({"1", "2"}, true);
        auto transportServiceMock = new TransportServiceMock();
        auto taskProviderMock = new TaskProviderMock();
        auto taskNode = new TaskNode(transportServiceMock, taskProviderMock, "127.0.0.1:3333", 10, 1, this);
        bool workDone = false;
        QObject::connect(taskNode, &TaskNode::stopped, this, [this, &workDone](bool success)
                         {
                            workDone = true;
                            QVERIFY(success); });
        taskProviderMock->m_tasks.append(testTask);
        QObject::connect(transportServiceMock, &TransportServiceMock::receivedCalcTask, this,
                         [this, transportServiceMock, testResult](const TransportServiceBase::PeerHandlerType peerHandler,
                                                                  const CalcTask task)
                         { (void)task; transportServiceMock->sendCalcResult(peerHandler, testResult); });
        taskNode->start();
        QTRY_VERIFY_WITH_TIMEOUT(workDone, 100);
        QVERIFY(!taskProviderMock->m_testFail);
        QVERIFY(!transportServiceMock->m_testFail);
        QCOMPARE(taskProviderMock->m_results.length(), 1);
        QCOMPARE(taskProviderMock->m_results.at(0).data, testResult.data);
        QCOMPARE(taskProviderMock->m_results.at(0).isMain, testResult.isMain);
    }
    void LoadErrorTest()
    {
        const auto testTask = CalcTask("sin(x)", {"0", "1"}, true);
        const auto testResult = CalcResult({"1", "2"}, true);
        auto transportServiceMock = new TransportServiceMock();
        auto taskProviderMock = new TaskProviderMock();
        taskProviderMock->m_loadError = true;
        auto taskNode = new TaskNode(transportServiceMock, taskProviderMock, "127.0.0.1:3333", 10, 1, this);
        bool workDone = false;
        QObject::connect(taskNode, &TaskNode::stopped, this, [this, &workDone](bool success)
                         {
                            workDone = true;
                            QVERIFY(!success); });
        taskProviderMock->m_tasks.append(testTask);
        QObject::connect(transportServiceMock, &TransportServiceMock::receivedCalcTask, this,
                         [this, transportServiceMock, testResult](const TransportServiceBase::PeerHandlerType peerHandler,
                                                                  const CalcTask task)
                         { (void)task; transportServiceMock->sendCalcResult(peerHandler, testResult); });
        taskNode->start();
        QTRY_VERIFY_WITH_TIMEOUT(workDone, 100);
        QVERIFY(!taskProviderMock->m_testFail);
        QVERIFY(!transportServiceMock->m_testFail);
        QCOMPARE(taskProviderMock->m_results.length(), 0);
    }
    void NoTasksTest()
    {
        const auto testTask = CalcTask("sin(x)", {"0", "1"}, true);
        const auto testResult = CalcResult({"1", "2"}, true);
        auto transportServiceMock = new TransportServiceMock();
        auto taskProviderMock = new TaskProviderMock();
        taskProviderMock->m_noTasks = true;
        auto taskNode = new TaskNode(transportServiceMock, taskProviderMock, "127.0.0.1:3333", 10, 1, this);
        bool workDone = false;
        QObject::connect(taskNode, &TaskNode::stopped, this, [this, &workDone](bool success)
                         {
                            workDone = true;
                            QVERIFY(!success); });
        taskProviderMock->m_tasks.append(testTask);
        QObject::connect(transportServiceMock, &TransportServiceMock::receivedCalcTask, this,
                         [this, transportServiceMock, testResult](const TransportServiceBase::PeerHandlerType peerHandler,
                                                                  const CalcTask task)
                         { (void)task; transportServiceMock->sendCalcResult(peerHandler, testResult); });
        taskNode->start();
        QTRY_VERIFY_WITH_TIMEOUT(workDone, 100);
        QVERIFY(!taskProviderMock->m_testFail);
        QVERIFY(!transportServiceMock->m_testFail);
        QCOMPARE(taskProviderMock->m_results.length(), 0);
    }
    void FormatErrorTest()
    {
        const auto testTask = CalcTask("sin(x)", {"0", "1"}, true);
        const auto testResult = CalcResult({"1", "2"}, true);
        auto transportServiceMock = new TransportServiceMock();
        auto taskProviderMock = new TaskProviderMock();
        taskProviderMock->m_formatError = true;
        auto taskNode = new TaskNode(transportServiceMock, taskProviderMock, "127.0.0.1:3333", 10, 1, this);
        bool workDone = false;
        QObject::connect(taskNode, &TaskNode::stopped, this, [this, &workDone](bool success)
                         {
                            workDone = true;
                            QVERIFY(!success); });
        taskProviderMock->m_tasks.append(testTask);
        QObject::connect(transportServiceMock, &TransportServiceMock::receivedCalcTask, this,
                         [this, transportServiceMock, testResult](const TransportServiceBase::PeerHandlerType peerHandler,
                                                                  const CalcTask task)
                         { (void)task; transportServiceMock->sendCalcResult(peerHandler, testResult); });
        taskNode->start();
        QTRY_VERIFY_WITH_TIMEOUT(workDone, 100);
        QVERIFY(!taskProviderMock->m_testFail);
        QVERIFY(!transportServiceMock->m_testFail);
        QCOMPARE(taskProviderMock->m_results.length(), 0);
    }
    void ConnectErrorTest()
    {
        const auto testTask = CalcTask("sin(x)", {"0", "1"}, true);
        const auto testResult = CalcResult({"1", "2"}, true);
        auto transportServiceMock = new TransportServiceMock();
        auto taskProviderMock = new TaskProviderMock();
        transportServiceMock->m_connectError = true;
        const int maxConnectAttempts = 10;
        auto taskNode = new TaskNode(transportServiceMock, taskProviderMock, "127.0.0.1:3333", maxConnectAttempts, 1, this);
        bool workDone = false;
        QObject::connect(taskNode, &TaskNode::stopped, this, [this, &workDone](bool success)
                         {
                            workDone = true;
                            QVERIFY(!success); });
        taskProviderMock->m_tasks.append(testTask);
        QObject::connect(transportServiceMock, &TransportServiceMock::receivedCalcTask, this,
                         [this, transportServiceMock, testResult](const TransportServiceBase::PeerHandlerType peerHandler,
                                                                  const CalcTask task)
                         { (void)task; transportServiceMock->sendCalcResult(peerHandler, testResult); });
        taskNode->start();
        QTRY_VERIFY_WITH_TIMEOUT(workDone, 100);
        QVERIFY(!taskProviderMock->m_testFail);
        QVERIFY(!transportServiceMock->m_testFail);
        QCOMPARE(taskProviderMock->m_results.length(), 0);
        QCOMPARE(transportServiceMock->m_connectAttempts, maxConnectAttempts);
    }
};