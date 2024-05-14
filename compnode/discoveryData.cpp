#include "discoveryData.h"

#include <QJsonArray>

QJsonObject DiscoveryData::serialize() const
{
    auto object = QJsonObject();
    object.insert("class", "DiscoveryData");
    object.insert("uuid", uuid);
    object.insert("connectInfo", QJsonArray::fromStringList(connectInfo));
    return object;
}

bool DiscoveryData::deserialize(const QJsonObject &input, std::function<void(DiscoveryData *)> handler)
{
    if (input["class"].toString() != "DiscoveryData")
        return false;
    auto task = new DiscoveryData(
        input["uuid"].toString(),
        QVariant(input["connectInfo"].toArray().toVariantList()).toStringList());
    handler(task);
    return true;
}