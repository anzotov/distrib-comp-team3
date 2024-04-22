#include "logMessageHandler.h"

#include <stdio.h>
#include <QString>

struct LogMessageLevel logMessageLevel;

void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (logMessageLevel.quiet)
    {
        return;
    }
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type)
    {
    case QtDebugMsg:
        if (logMessageLevel.debug)
        {
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        }
        break;
    case QtInfoMsg:
        if (logMessageLevel.verbose)
        {
            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        }
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    }
}