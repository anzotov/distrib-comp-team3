#pragma once

#include "../common/calcTask.h"
#include "calculatorService.h"
#include <QObject>
#include <QStringList>

class CompNode : public QObject
{
Q_OBJECT

public:
    explicit CompNode(QObject *parent = nullptr);

public slots:
    void processTask(const CalcTask& task);

signals:
    void taskCompleted(const QStringList& result);

private:
    CalculatorService m_calculatorService;
};
