#include "fileTaskProvider.h"

#include <QtDebug>
#include <stdexcept>

FileTaskProvider::FileTaskProvider(const QString &inputFileName, const QString &outputFileName, QObject *parent)
    : TaskProvider(parent), m_inputFile(inputFileName), m_outputFile(outputFileName)
{
    qDebug() << QStringLiteral("FileTaskProvider(%1, %2)").arg(inputFileName).arg(outputFileName);
    if (inputFileName == "-")
    {
        if (!m_inputFile.open(stdin, QIODevice::ReadOnly | QIODevice::Text))
        {
            throw std::runtime_error("FileTaskProvider: Error opening standard input");
        }
    }
    else
    {
        if (!m_inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            throw std::runtime_error(qPrintable(QStringLiteral("FileTaskProvider: Error opening input file %1").arg(inputFileName)));
        }
    }

    if (outputFileName == "-")
    {
        if (!m_outputFile.open(stdout, QIODevice::WriteOnly | QIODevice::Text))
        {
            throw std::runtime_error("FileTaskProvider: Error opening standard output");
        }
    }
    else
    {
        if (!m_outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            throw std::runtime_error(qPrintable(QStringLiteral("FileTaskProvider: Error opening output file %1").arg(inputFileName)));
        }
    }
}

void FileTaskProvider::loadNextTask()
{
    qDebug() << "FileTaskProvider: loadNextTask()";
    if (m_taskLoaded)
    {
        qInfo() << "FileTaskProvider: no tasks availvable";
        emit noTasksAvailable();
        return;
    }

    CalcTask task;
    {
        QTextStream inputStream(&m_inputFile);
        QString line;
        bool firstString = true;
        while (inputStream.readLineInto(&line) && !line.isEmpty())
        {
            if (firstString)
            {
                task.function = line;
                firstString = false;
                continue;
            }
            task.data << line;
        }

        if (task.function.isEmpty() || task.data.isEmpty())
        {
            qCritical() << "FileTaskProvider: Task load error";
            emit taskLoadError();
            return;
        }
    }

    m_inputFile.close();
    m_taskLoaded = true;
    qInfo() << "FileTaskProvider: Task loaded successfully";
    emit taskLoadDone(task);
}

void FileTaskProvider::formatResult(const CalcResult &result)
{
    qDebug() << "FileTaskProvider: formatResult()";
    if (m_resultFormatted)
    {
        emit resultFormatError(result);
        return;
    }

    {
        QTextStream outputSteam(&m_outputFile);

        for (auto &line : result.data)
        {
            outputSteam << line << '\n';
        }
    }

    m_outputFile.close();
    m_resultFormatted = true;
    qInfo() << "FileTaskProvider: result formatted successfully";
    emit resultFormatDone();
}
