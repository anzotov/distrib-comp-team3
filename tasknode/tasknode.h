#pragma once

#include <QObject>
#include <QString>

class TaskNode : public QObject
{
    Q_OBJECT

public:
    TaskNode(QObject *parent,
             const QString ip, const QString port, const QString input, const QString output);
};