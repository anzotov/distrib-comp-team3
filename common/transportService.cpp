#include "transportService.h"
#include "../compnode/compnode.h"

static CompNode *compNode = nullptr;

TransportService::TransportService(QObject *parent) : QObject(parent)
{
    compNode = new CompNode(this);
    QObject::connect(compNode, &CompNode::taskCompleted, this, [this](const QStringList &result)
                     { return receiveMainResult(result, 0); });
}

void TransportService::openConnection(const QHostAddress IP, const quint64 port)
{
    emit newConnection(IP, port, 0);
}

void TransportService::closeConnection(const quint64 socketNum)
{
}

void TransportService::sendMainTask(const CalcTask &task, const quint64 socketNum)
{
    compNode->processTask(task);
}

void TransportService::sendMainResult(const QStringList &result, const quint64 socketNum)
{
}

void TransportService::startListening(const quint64 port)
{
}

void TransportService::stopListening()
{
}
