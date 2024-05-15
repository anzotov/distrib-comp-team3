#pragma once

#include "../common/calcTask.h"
#include "../common/calcResult.h"

#include <QObject>

class TaskProvider : public QObject
{
    Q_OBJECT
public:
    TaskProvider(QObject *parent = nullptr) : QObject(parent) {}
    
    virtual void loadNextTask() = 0;
    virtual void formatResult(const CalcResult &result) = 0;
signals:
    void taskLoadDone(const CalcTask task);
    void taskLoadError();
    void noTasksAvailable();
    void resultFormatDone();
    void resultFormatError(CalcResult result);
};