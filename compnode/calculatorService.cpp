#include "../common/calcTask.h"
#include "calculatorService.h"

CalculatorService::CalculatorService(QObject *parent) : QObject(parent)
{
    m_jsEngine.globalObject().setProperty("sin", m_jsEngine.evaluate("f=function(x) { return Math.sin(x); }"));
    m_jsEngine.globalObject().setProperty("cos", m_jsEngine.evaluate("f=function(x) { return Math.cos(x); }"));
    m_jsEngine.globalObject().setProperty("tg", m_jsEngine.evaluate("f=function(x) { return Math.tan(x); }"));
    m_jsEngine.globalObject().setProperty("ctg", m_jsEngine.evaluate("f=function(x) { return 1/Math.tan(x); }"));
}

QStringList CalculatorService::calculate(const CalcTask &task)
{
    auto result = QStringList();

    for (auto i = 0; i < task.data.length(); ++i)
    {
        m_jsEngine.globalObject().setProperty("x", m_jsEngine.evaluate(task.data.at(i)));
        result << m_jsEngine.evaluate(task.function).toString();
    }
    return result;
}
