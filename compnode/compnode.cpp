#include <QObject>
#include "compnode.h"

CompNode::CompNode(QObject *parent) : QObject(parent)
{
    connect(this, &CompNode::taskCompleted, this, [=](const Result& result) {
qDebug() << "Task completed. Result:" << result;
});
}

void CompNode::processTask(const Task& task)
{
    Result result = calculatorService.calculate(task);
    emit taskCompleted(result);
}
