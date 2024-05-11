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
    QString toQString() const
    {
        return QStringLiteral("CalcTask(%1, {%2}, %3)").arg(function).arg(data.join(",")).arg(isMain);
    }
    bool operator==(const CalcTask &other) const
    {
        return this == &other || (function == other.function && data == other.data && isMain == other.isMain);
    }
    bool operator!=(const CalcTask &other) const
    {
        return !(*this == other);
    }
};