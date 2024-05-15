#pragma once

#include <QtGlobal>

extern struct LogMessageLevel
{
public:
    bool quiet = false;
    bool verbose = false;
    bool debug = false;
} logMessageLevel;

extern void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);