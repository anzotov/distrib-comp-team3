#pragma once

#include "../tasknode/fileTaskProvider.h"

#include <QObject>
#include <QTest>

class FileTaskProviderTests : public QObject
{
    Q_OBJECT
private:
    void createTaskFile()
    {
        QFile file(m_inputFileName);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream stream(&file);
        stream << "sin(x)\n";
        stream << "0\n";
        stream << "1\n";
        stream << "2\n";
    }
    void createErrorTaskFile()
    {
        QFile file(m_inputFileName);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream stream(&file);
        stream << "sin(x)\n";
    }
    void createEmptyTaskFile()
    {
        QFile file(m_inputFileName);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream stream(&file);
    }

    void checkOutputFile()
    {
        QFile file(m_outputFileName);
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
        QTextStream stream(&file);
        stream.readLine() == "1";
        stream.readLine() == "2";
        stream.readLine() == "3";
    }

    QString m_inputFileName = "input.txt";
    QString m_outputFileName = "output.txt";

private slots:
    void cleanup()
    {
        QFile::remove(m_inputFileName);
        QFile::remove(m_outputFileName);
    }

    void taskLoadPositiveTest()
    {
        createTaskFile();
        FileTaskProvider provider(m_inputFileName, m_outputFileName);
        bool done = false;
        QObject::connect(&provider, &FileTaskProvider::taskLoadDone, this, [this, &done](const CalcTask task)
                         {
            QCOMPARE(task.function, "sin(x)");
            QCOMPARE(task.data.length(), 3);
            QCOMPARE(task.data.at(0), "0");
            QCOMPARE(task.data.at(1), "1");
            QCOMPARE(task.data.at(2), "2");
            done = true; });
        QObject::connect(&provider, &FileTaskProvider::taskLoadError, this, [this]()
                         { QFAIL("taskLoadError"); });
        QObject::connect(&provider, &FileTaskProvider::noTasksAvailable, this, [this]()
                         { QFAIL("noTasksAvailable"); });
        provider.loadNextTask();
        QTRY_VERIFY_WITH_TIMEOUT(done, 100);
    }

    void noTasksAvailableTest()
    {
        createTaskFile();
        FileTaskProvider provider(m_inputFileName, m_outputFileName);
        bool done = false, noTasks = false;
        QObject::connect(&provider, &FileTaskProvider::taskLoadDone, this, [this, &done](const CalcTask task)
                         { (void)task; done = true; });
        QObject::connect(&provider, &FileTaskProvider::taskLoadError, this, [this]()
                         { QFAIL("taskLoadError"); });
        QObject::connect(&provider, &FileTaskProvider::noTasksAvailable, this, [this, &noTasks]()
                         { noTasks = true; });
        provider.loadNextTask();
        QTRY_VERIFY_WITH_TIMEOUT(done, 100);
        QVERIFY(!noTasks);
        done = false;
        provider.loadNextTask();
        QTRY_VERIFY_WITH_TIMEOUT(noTasks, 100);
        QVERIFY(!done);
    }

    void taskLoadErrorFileTest()
    {
        createErrorTaskFile();
        FileTaskProvider provider(m_inputFileName, m_outputFileName);
        bool done = false;
        QObject::connect(&provider, &FileTaskProvider::taskLoadDone, this, [this](const CalcTask task)
                         { (void)task; QFAIL("taskLoadDone"); });
        QObject::connect(&provider, &FileTaskProvider::taskLoadError, this, [this, &done]()
                         { done = true; });
        QObject::connect(&provider, &FileTaskProvider::noTasksAvailable, this, [this]()
                         { QFAIL("noTasksAvailable"); });
        provider.loadNextTask();
        QTRY_VERIFY_WITH_TIMEOUT(done, 100);
    }

    void taskLoadErrorEmptyFileTest()
    {
        createEmptyTaskFile();
        FileTaskProvider provider(m_inputFileName, m_outputFileName);
        bool done = false;
        QObject::connect(&provider, &FileTaskProvider::taskLoadDone, this, [this](const CalcTask task)
                         { (void)task; QFAIL("taskLoadDone"); });
        QObject::connect(&provider, &FileTaskProvider::taskLoadError, this, [this, &done]()
                         { done = true; });
        QObject::connect(&provider, &FileTaskProvider::noTasksAvailable, this, [this]()
                         { QFAIL("noTasksAvailable"); });
        provider.loadNextTask();
        QTRY_VERIFY_WITH_TIMEOUT(done, 100);
    }

    void taskLoadErrorNoFileTest()
    {
        QVERIFY_EXCEPTION_THROWN(FileTaskProvider(m_inputFileName, m_outputFileName), std::runtime_error);
    }

    void resultFormatTest()
    {
        createTaskFile();
        FileTaskProvider provider(m_inputFileName, m_outputFileName);
        bool done = false;
        QObject::connect(&provider, &FileTaskProvider::resultFormatDone, this, [this, &done]()
                         { done = true; });
        QObject::connect(&provider, &FileTaskProvider::resultFormatError, this, [this](CalcResult result)
                         { (void)result; QFAIL("resultFormatError"); });
        provider.formatResult(CalcResult({"1", "2", "3"}, true));
        QTRY_VERIFY_WITH_TIMEOUT(done, 100);
    }

    void resultFormatErrorTest()
    {
        createTaskFile();
        FileTaskProvider provider(m_inputFileName, m_outputFileName);
        bool done = false, error = false;
        QObject::connect(&provider, &FileTaskProvider::resultFormatDone, this, [this, &done]()
                         { done = true; });
        QObject::connect(&provider, &FileTaskProvider::resultFormatError, this, [this, &error](CalcResult result)
                         { (void)result; error = true; });
        provider.formatResult(CalcResult({"1", "2", "3"}, true));
        QTRY_VERIFY_WITH_TIMEOUT(done, 100);
        done = false;
        QVERIFY(!error);
        provider.formatResult(CalcResult({"1", "2", "3"}, true));
        QTRY_VERIFY_WITH_TIMEOUT(error, 100);
        QVERIFY(!done);
    }
};