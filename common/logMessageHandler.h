#pragma once

#include <QtGlobal>

extern struct LogMessageLevel
{
public:
    bool quiet;
    bool verbose;
    bool debug;
} logMessageLevel;

extern void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);