#include "tasknode.h"
#include <QtDebug>
#include <QFile>

TaskNode::TaskNode(QObject *parent, const QString &ip, const QString &port, const QString &input, const QString &output) : QObject(parent)
{
    qDebug() << "TaskNode(" << ip << "," << port << "," << input << "," << output << ")";

    parseInput(input);

    if (output == "-")
    {
        m_outputSteam = new QTextStream(stdout, QIODevice::WriteOnly);
    }
    else
    {
        auto outputFile = new QFile(output, this);
        if (!outputFile->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qFatal("Error opening output file %s", qPrintable(output));
        }
        m_outputSteam = new QTextStream(outputFile);
    }
}

TaskNode::~TaskNode()
{
    qDebug() << "~TaskNode()";
    delete m_calcTask;
    delete m_outputSteam;
}

void TaskNode::parseInput(const QString &input)
{
    QFile inputFile(input);
    if (input == "-")
    {
        if (!inputFile.open(stdin, QIODevice::ReadOnly | QIODevice::Text))
        {
            qFatal("Error opening standard input");
        }
    }
    else
    {
        if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qFatal("Error opening input file %s", qPrintable(input));
        }
    }
    QTextStream inputStream(&inputFile);

    m_calcTask = new CalcTask();
    QString line;
    bool firstString = true;
    while (inputStream.readLineInto(&line) && !line.isEmpty())
    {
        if (firstString)
        {
            m_calcTask->function = line;
            firstString = false;
            continue;
        }
        m_calcTask->data << line;
    }
    qDebug() << "m_calcTask->function: " << m_calcTask->function;
    qDebug() << "m_calcTask->data: " << m_calcTask->data;

    if (m_calcTask->function.isEmpty() || m_calcTask->data.isEmpty())
    {
        qFatal("Incorrect input data!");
    }
}
