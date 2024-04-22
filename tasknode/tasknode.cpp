#include "tasknode.h"
#include <QtDebug>

TaskNode::TaskNode(QObject *parent, const QString ip, const QString port, const QString input, const QString output)
{
    qDebug() << "TaskNode(" << ip << port << input << output;
}
