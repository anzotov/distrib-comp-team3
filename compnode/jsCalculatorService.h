#pragma once

#include "calculatorServiceBase.h"

#include <QJSEngine>

class JsCalculatorService final : public CalculatorServiceBase
{
public:
    explicit JsCalculatorService(QObject *parent = nullptr);
    void calculate(const CalcTask &task) override final;
    void stop();

private:
    QJSEngine m_jsEngine;
    bool m_stop = true;
};