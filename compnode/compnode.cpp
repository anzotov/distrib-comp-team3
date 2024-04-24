#include "compnode.h"

#include <QObject>
#include "QtDebug"

CompNode::CompNode(QObject *parent) : QObject(parent)
{
    connect(this, &CompNode::taskCompleted, this, [=](const QStringList &result)
            { qDebug() << "Task completed. Result:" << result; });
}

void CompNode::processTask(const CalcTask &task)
{
    auto result = m_calculatorService.calculate(task);
    emit taskCompleted(result);
}
