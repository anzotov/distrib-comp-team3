#pragma once

#include "../compnode/calculatorServiceBase.h"
#include <functional>

struct CalculatorServiceMock : CalculatorServiceBase
{
    CalculatorServiceMock(QObject *parent = nullptr)
        : CalculatorServiceBase(parent)
    {
        if (m_constructor)
            m_constructor(this);
    }
    ~CalculatorServiceMock()
    {
        if (m_destructor)
            m_destructor(this);
    }

    void calculate(const CalcTask &task) override final
    {
        ++m_calculateCount;
        if (m_calculate)
            m_calculate(this, task);
    }
    void stop() override final
    {
        ++m_stopCount;
        if (m_stop)
            m_stop(this);
    }

    std::function<void(CalculatorServiceMock *)> m_constructor;
    std::function<void(CalculatorServiceMock *, const CalcTask &)> m_calculate;
    int m_calculateCount = 0;
    std::function<void(CalculatorServiceMock *)> m_stop;
    int m_stopCount = 0;
    std::function<void(CalculatorServiceMock *)> m_destructor;
};
