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
};