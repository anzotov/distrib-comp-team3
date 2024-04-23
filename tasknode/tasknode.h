#pragma once

#include "../common/calcTask.h"
#include "../common/transportService.h"

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

signals:
    void taskDone(bool success);

private slots:
    void receiveMainResult(const QStringList &result, const quint64 socketNum);
    void newConnection(const QHostAddress IP, const quint64 port, const quint64 socketNum);

private:
    void parseInput(const QString &input);
    void openOutput(const QString &output);
    QTextStream *m_outputSteam = nullptr;
    CalcTask m_calcTask;
    TransportService m_transportService;
};