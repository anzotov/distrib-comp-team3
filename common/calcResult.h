#pragma once

#include <QString>
#include <QStringList>
#include <functional>
#include <QJsonObject>

struct CalcResult final
{
    CalcResult() = default;
    CalcResult(const QStringList &data, bool isMain);
    QStringList data;
    bool isMain = false;

    QJsonObject serialize() const;
    static bool deserialize(const QJsonObject &input, std::function<void(CalcResult *)> handler);

    QString toQString() const
    {
        return QStringLiteral("CalcResult({%1}, %2)").arg(data.join(",")).arg(isMain);
    }

    bool operator==(const CalcResult &other) const
    {
        return this == &other || (data == other.data && isMain == other.isMain);
    }
    
    bool operator!=(const CalcResult &other) const
    {
        return !(*this == other);
    }
};