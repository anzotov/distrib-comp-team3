#pragma once

#include <QObject>
#include "calculatorService.h"

class CompNode : public QObject
{
Q_OBJECT

public:
    explicit CompNode(QObject *parent = nullptr);

public slots:
    void processTask(const Task& task);

signals:
    void taskCompleted(const Result& result);

private:
    CalculatorService calculatorService;
};
