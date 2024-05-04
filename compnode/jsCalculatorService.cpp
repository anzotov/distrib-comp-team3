#include "jsCalculatorService.h"

#include <QCoreApplication>
#include <chrono>

using namespace std::literals;

JsCalculatorService::JsCalculatorService(QObject *parent) : CalculatorService(parent)
{
    m_jsEngine.globalObject().setProperty("sin", m_jsEngine.evaluate("f=function(x) { return Math.sin(x); }"));
    m_jsEngine.globalObject().setProperty("cos", m_jsEngine.evaluate("f=function(x) { return Math.cos(x); }"));
    m_jsEngine.globalObject().setProperty("tg", m_jsEngine.evaluate("f=function(x) { return Math.tan(x); }"));
    m_jsEngine.globalObject().setProperty("ctg", m_jsEngine.evaluate("f=function(x) { return 1/Math.tan(x); }"));
}

void JsCalculatorService::calculate(const CalcTask &task)
{
    m_stop = false;
    auto result = CalcResult();
    result.isMain = task.isMain;

    auto start_time = std::chrono::steady_clock::now();
    for (auto i = 0; i < task.data.length(); ++i)
    {
        m_jsEngine.globalObject().setProperty("x", m_jsEngine.evaluate(task.data.at(i)));
        result.data << m_jsEngine.evaluate(task.function).toString();
        if (std::chrono::steady_clock::now() - start_time > 2s)
        {
            QCoreApplication::processEvents();
            if (m_stop)
                return;
        }
    }
    emit calcDone(result);
}

void JsCalculatorService::stop()
{
    m_stop = true;
}
