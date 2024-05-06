#include "calcTask.h"

#include <QJsonArray>

CalcTask::CalcTask(const QString &function, const QStringList &data, bool isMain) : function(function), data(data), isMain(isMain) {}

QJsonObject CalcTask::serialize() const
{
    auto object = QJsonObject();
    object.insert("class", "CalcTask");
    object.insert("function", function);
    object.insert("data", QJsonArray::fromStringList(data));
    object.insert("isMain", isMain);
    return object;
}

bool CalcTask::deserialize(const QJsonObject &input, std::function<void(CalcTask *)> handler)
{
    if (input["class"].toString() != "CalcTask")
        return false;
    auto task = new CalcTask(
        input["function"].toString(),
        QVariant(input["data"].toArray().toVariantList()).toStringList(),
        input["isMain"].toBool());
    handler(task);
    return true;
}