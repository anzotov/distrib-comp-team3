#pragma once

#include "../common/calcTask.h"

#include <QObject>
#include <QString>
#include <QTextStream>

class TaskNode final : public QObject
{
    Q_OBJECT

public:
    TaskNode(QObject *parent,
             const QString &ip, const QString &port, const QString &input, const QString &output);
    ~TaskNode();

private:
    void parseInput(const QString &input);
    CalcTask *m_calcTask = nullptr;
    QTextStream *m_outputSteam = nullptr;
};