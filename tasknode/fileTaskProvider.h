#pragma once

#include "taskProvider.h"

#include <QString>
#include <QFile>

class FileTaskProvider final : public TaskProvider
{
public:
    FileTaskProvider(const QString &inputFileName, const QString &outputFileName, QObject *parent = nullptr);
    void loadNextTask() override final;
    void formatResult(const CalcResult &result) override final;
private:
    QFile m_inputFile;
    QFile m_outputFile;
    bool m_taskLoaded = false;
    bool m_resultFormatted = false;
};