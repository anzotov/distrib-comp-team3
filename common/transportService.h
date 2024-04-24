#pragma once

#include "../common/calcTask.h"
#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QHostAddress>
#include <QString>
#include <QPointer>

class TransportService : public QObject
{
    Q_OBJECT
public:
    explicit TransportService(QObject *parent = nullptr);
signals:
    void newConnection(const QHostAddress IP, const quint64 port, const quint64 socketNum);
    void receiveMainTask(const CalcTask &task, const quint64 socketNum);
    void receiveMainResult(const QStringList &result, const quint64 socketNum);
public slots:
    void openConnection(const QHostAddress IP, const quint64 port);
    void closeConnection(const quint64 socketNum);
    void sendMainTask(const CalcTask &task, const quint64 socketNum);
    void sendMainResult(const QStringList &result, const quint64 socketNum);
    void startListening(const quint64 port);
    void stopListening();

private:
    QTcpServer m_server;
    QVector<QPointer<QTcpSocket>> m_socket;
    QHostAddress m_address;
    quint64 m_port;
};