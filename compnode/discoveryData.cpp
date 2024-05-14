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
    if (input["class"].toString() != "DiscoveryData" || !input["uuid"].isString() || !input["connectInfo"].isArray())
        return false;
    auto object = new DiscoveryData(
        input["uuid"].toString(),
        QVariant(input["connectInfo"].toArray().toVariantList()).toStringList());
    handler(object);
    return true;
}
