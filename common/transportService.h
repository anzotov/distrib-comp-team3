#pragma once

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QHostAddress>
#include <QString>
#include <calcTask.h>

class transportService : public QObject
{
    Q_OBJECT
public:
    explicit transportService(QObject *parent = nullptr);


signals:
    void newConnection(const QHostAddress IP, const quint64 port);
    void receiveMainTask(const CalcTask& task, const quint64 socketNum);
    void receiveResultTask(const QStringList& result, const quint64 socketNum);
public slots:
    void openConnection(const QHostAddress IP, const quint64 port);
    void closeConnection(const QHostAddress IP, const quint64 port);
    void sendMainTask(const CalcTask& task, const quint64 socketNum);
    void sendMainResult(const QStringList& result, const quint64 socketNum);
private:
    QTcpServer m_server;
    QVector<QPointer<QTcpSocket>> m_socket;
    QHostAddress m_address;
    quint64 m_port;
};
