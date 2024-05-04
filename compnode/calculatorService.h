#pragma once

#include "../common/calcTask.h"
#include "../common/calcResult.h"

#include <QObject>

class CalculatorService : public QObject
{
    Q_OBJECT
public:
    explicit CalculatorService(QObject *parent) : QObject(parent) {}
    virtual ~CalculatorService() = default;

signals:
    void calcDone(CalcResult result);

public:
    virtual void calculate(const CalcTask &task) = 0;
    virtual void stop() = 0;
};