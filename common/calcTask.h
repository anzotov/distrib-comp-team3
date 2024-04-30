#pragma once

#include <QString>
#include <QStringList>
#include <functional>
#include <QJsonObject>

struct CalcTask final
{
    CalcTask() = default;
    CalcTask(const QString &function, const QStringList &data, bool isMain);
    QString function;
    QStringList data;
    bool isMain = false;

    QJsonObject serialize() const;
    static bool deserialize(const QJsonObject &input, std::function<void(CalcTask *)> handler);
};