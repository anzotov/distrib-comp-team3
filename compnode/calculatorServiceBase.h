#pragma once

#include "../common/calcTask.h"
#include "../common/calcResult.h"

#include <QObject>

class CalculatorServiceBase : public QObject
{
    Q_OBJECT
public:
    CalculatorServiceBase(QObject *parent) : QObject(parent) {}

signals:
    void calcDone(CalcResult result);

public:
    virtual void calculate(const CalcTask &task) = 0;
    virtual void stop() = 0;
};