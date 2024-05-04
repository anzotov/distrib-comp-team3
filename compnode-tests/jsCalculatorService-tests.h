#pragma once

#include "../compnode/jsCalculatorService.h"
#include "../common/calcTask.h"
#include "../common/calcResult.h"

#include <QObject>
#include <QTest>

class jsCalculatorServiceTests : public QObject
{
    Q_OBJECT

private slots:
    void Test1()
    {
        taskTest(false);
    }
    void Positiveest2()
    {
        taskTest(true);
    }

private:
    void taskTest(bool isMain)
    {
        const CalcTask task("2+x**3", {
                                          "0",
                                          "-1",
                                          "1",
                                          "2",
                                          "-2",
                                      },
                            isMain);

        JsCalculatorService calculator;
        bool calcDone = false;
        QObject::connect(&calculator, &JsCalculatorService::calcDone, this, [this, &task, &calcDone](CalcResult result)
                         {
                            QCOMPARE(result.isMain, task.isMain);
                            QCOMPARE(result.data.size(), 5);
                            QCOMPARE(result.data.at(0), "2");
                            QCOMPARE(result.data.at(1), "1");
                            QCOMPARE(result.data.at(2), "3");
                            QCOMPARE(result.data.at(3), "10");
                            QCOMPARE(result.data.at(4), "-6"); 
                            calcDone = true; });

        calculator.calculate(task);
        QVERIFY(calcDone);
    }
};