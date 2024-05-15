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
        if (logMessageLevel.verbose || logMessageLevel.debug)
        {
            fprintf(stderr, "%s\n", localMsg.constData());
        }
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", localMsg.constData());
        break;
    }
}