#pragma once

#include "../compnode/chunkerService.h"
#include "../compnode/calculatorServiceBase.h"

#include <QObject>
#include <QTest>
#include <functional>

struct CalculatorServiceMock : CalculatorServiceBase
{
    CalculatorServiceMock(QObject *parent = nullptr)
        : CalculatorServiceBase(parent)
    {
        if (m_constructor)
            m_constructor(this);
    }
    ~CalculatorServiceMock()
    {
        if (m_destructor)
            m_destructor(this);
    }

    void calculate(const CalcTask &task) override final
    {
        ++m_calculateCount;
        if (m_calculate)
            m_calculate(this, task);
    }
    void stop() override final
    {
        ++m_stopCount;
        if (m_stop)
            m_stop(this);
    }

    std::function<void(CalculatorServiceMock *)> m_constructor;
    std::function<void(CalculatorServiceMock *, const CalcTask &)> m_calculate;
    int m_calculateCount = 0;
    std::function<void(CalculatorServiceMock *)> m_stop;
    int m_stopCount = 0;
    std::function<void(CalculatorServiceMock *)> m_destructor;
};

class ChunkerServiceTestCase : public QObject
{
    Q_OBJECT

private slots:
    void TestTaskTest()
    {
        auto calculatorServiceMock = new CalculatorServiceMock;
        calculatorServiceMock->m_calculate =
            [&](CalculatorServiceMock *, const CalcTask &task)
        {
            QTest::qWait(1000);
            emit calculatorServiceMock->calcDone(CalcResult({"0", "1"}, task.isMain));
        };
        ChunkerService chunkerService(calculatorServiceMock);
        int readyCount = 0;
        QObject::connect(&chunkerService, &ChunkerService::ready, this,
                         [&](const QString compPower)
                         {
                             ++readyCount;
                             qDebug() << "CompPower: " << compPower;
                             double power = compPower.toDouble();
                             QVERIFY(power < 1.2e9 && power > 0.8e9);
                         });
        chunkerService.start();
        QTRY_COMPARE(calculatorServiceMock->m_calculateCount, 1);
        QTRY_COMPARE(calculatorServiceMock->m_stopCount, 0);
        QTRY_COMPARE(readyCount, 1);
        chunkerService.stop();
        QTRY_COMPARE(calculatorServiceMock->m_stopCount, 1);
    }
    void OneNodeChunkedTaskTest()
    {
        OneNodeTaskTest(false);
    }
    void OneNodeMainTaskTest()
    {
        OneNodeTaskTest(true);
    }
    void TwoNodesTest()
    {
        CalcTask test_task("sin(x)", {"0", "1"}, true);
        CalcResult test_result({"2", "3"}, test_task.isMain);

        CalcTask test_peerChunk("sin(x)", {"0"}, false);
        CalcResult test_peerChunkResult({"2"}, test_peerChunk.isMain);

        CalcTask test_myChunk("sin(x)", {"1"}, false);
        CalcResult test_myChunkResult({"3"}, test_myChunk.isMain);

        PeerHandlerType test_taskNodeHandler = "123";
        PeerHandlerType test_peerHandler = "125";
        QString test_peerPower = "1000000000";
        QString test_peerSpeed = "9000000000";
        auto calculatorServiceMock = new CalculatorServiceMock;
        calculatorServiceMock->m_calculate =
            [&](CalculatorServiceMock *self, const CalcTask &task)
        {
            if (self->m_calculateCount == 1)
            {
                QTest::qWait(1000);
                emit calculatorServiceMock->calcDone(CalcResult({"0", "1"}, task.isMain));
                return;
            }
            QCOMPARE(task, test_myChunk);
            emit calculatorServiceMock->calcDone(test_myChunkResult);
        };
        ChunkerService chunkerService(calculatorServiceMock);
        int calcResultCount = 0;
        QObject::connect(&chunkerService, &ChunkerService::calcResult, this,
                         [&](const PeerHandlerType peerHandler, const CalcResult result)
                         {
                             ++calcResultCount;
                             QCOMPARE(peerHandler, test_taskNodeHandler);
                             QCOMPARE(result, test_result);
                         });
        int sendChunkedTaskCount = 0;
        QObject::connect(&chunkerService, &ChunkerService::sendChunkedTask, this,
                         [&](const PeerHandlerType peerHandler, const CalcTask task)
                         {
                             ++sendChunkedTaskCount;
                             QCOMPARE(peerHandler, test_peerHandler);
                             QCOMPARE(task, test_peerChunk);
                             chunkerService.processChunkedResult(peerHandler, test_peerChunkResult);
                         });
        chunkerService.start();
        QTRY_COMPARE(calculatorServiceMock->m_calculateCount, 1);
        chunkerService.updatePeers({PeerInfo{test_peerHandler, test_peerPower, test_peerSpeed}});
        chunkerService.calculateTask(test_taskNodeHandler, test_task);
        QTRY_COMPARE(calculatorServiceMock->m_calculateCount, 2);
        QTRY_COMPARE(sendChunkedTaskCount, 1);
        QTRY_COMPARE(calcResultCount, 1);
    }

private:
    void OneNodeTaskTest(bool isMain)
    {
        CalcTask test_task("sin(x)", {"0", "1"}, isMain);
        CalcResult test_result({"2", "3"}, test_task.isMain);
        PeerHandlerType test_taskNodeHandler = "123";
        auto calculatorServiceMock = new CalculatorServiceMock;
        calculatorServiceMock->m_calculate =
            [&](CalculatorServiceMock *self, const CalcTask &task)
        {
            if (self->m_calculateCount == 1)
            {
                emit calculatorServiceMock->calcDone(CalcResult({"0", "1"}, task.isMain));
                return;
            }
            QCOMPARE(task, test_task);
            emit calculatorServiceMock->calcDone(test_result);
        };
        ChunkerService chunkerService(calculatorServiceMock);
        int calcResultCount = 0;
        QObject::connect(&chunkerService, &ChunkerService::calcResult, this,
                         [&](const PeerHandlerType peerHandler, const CalcResult result)
                         {
                             ++calcResultCount;
                             QCOMPARE(peerHandler, test_taskNodeHandler);
                             QCOMPARE(result, test_result);
                         });
        chunkerService.start();
        QTRY_COMPARE(calculatorServiceMock->m_calculateCount, 1);
        chunkerService.calculateTask(test_taskNodeHandler, test_task);
        QTRY_COMPARE(calculatorServiceMock->m_calculateCount, 2);
        QTRY_COMPARE(calcResultCount, 1);
    }
};