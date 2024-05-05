#pragma once

#include "calculatorServiceBase.h"

#include <QJSEngine>
#include <QAtomicInteger>

class JsCalculatorService final : public CalculatorServiceBase
{
public:
    explicit JsCalculatorService(QObject *parent = nullptr);
    void calculate(const CalcTask &task) override final;
    void stop();

private:
    QJSEngine m_jsEngine;
    QAtomicInteger<bool> m_stop = true;
};