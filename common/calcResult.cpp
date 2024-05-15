#include "calcResult.h"

#include <QJsonArray>

CalcResult::CalcResult(const QStringList &data, bool isMain) : data(data), isMain(isMain) {}

QJsonObject CalcResult::serialize() const
{
    auto object = QJsonObject();
    object.insert("class", "CalcResult");
    object.insert("data", QJsonArray::fromStringList(data));
    object.insert("isMain", isMain);
    return object;
}

bool CalcResult::deserialize(const QJsonObject &input, std::function<void(CalcResult *)> handler)
{
    if (input["class"].toString() != "CalcResult" || !input["data"].isArray() || !input["isMain"].isBool())
        return false;
    auto object = new CalcResult(
        QVariant(input["data"].toArray().toVariantList()).toStringList(),
        input["isMain"].toBool());
    handler(object);
    return true;
}
