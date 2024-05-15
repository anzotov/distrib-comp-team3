#pragma once

#include "../tasknode/taskProvider.h"

#include <functional>

struct TaskProviderMock final : TaskProvider
{
    TaskProviderMock(QObject *parent = nullptr) : TaskProvider(parent)
    {
        if (m_constructor)
            m_constructor(this);
    }
    ~TaskProviderMock()
    {
        if (m_destructor)
            m_destructor(this);
    }
    void loadNextTask() override final
    {
        ++m_loadNextTaskCount;
        if (m_loadNextTask)
            m_loadNextTask(this);
    }
    void formatResult(const CalcResult &result) override final
    {
        ++m_formatResultCount;
        if (m_formatResult)
            m_formatResult(this, result);
    }

    std::function<void(TaskProviderMock *)> m_constructor;
    std::function<void(TaskProviderMock *)> m_destructor;
    std::function<void(TaskProviderMock *)> m_loadNextTask;
    int m_loadNextTaskCount = 0;
    std::function<void(TaskProviderMock *, const CalcResult &)> m_formatResult;
    int m_formatResultCount = 0;
};
