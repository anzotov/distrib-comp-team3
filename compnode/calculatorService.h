#pragma once

#include <QObject>
#include <QStringList>
#include <QJSEngine>

struct CalcTask;

class CalculatorService : public QObject
{
    Q_OBJECT

public:
    explicit CalculatorService(QObject *parent = nullptr);
    QStringList calculate(const CalcTask &task);

private:
    QJSEngine m_jsEngine;
};