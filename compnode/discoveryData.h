#pragma once

#include <QString>
#include <QStringList>
#include <functional>
#include <QJsonObject>

struct DiscoveryData final
{
    QString uuid;
    QStringList connectInfo;

    DiscoveryData() = default;
    DiscoveryData(QString uuid, QStringList connectInfo) : uuid(uuid), connectInfo(connectInfo) {}
    QJsonObject serialize() const;
    static bool deserialize(const QJsonObject &input, std::function<void(DiscoveryData *)> handler);
    QString toQString()
    {
        return QStringLiteral("DiscoveryData(%1, {%2})").arg(uuid).arg(connectInfo.join(","));
    }
};