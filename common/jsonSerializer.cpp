#include "jsonSerializer.h"

#include <QJsonDocument>
#include <stdexcept>

QByteArray JsonSerializer::serialize_array(const QJsonObject &intermediate) const
{
    return QJsonDocument(intermediate).toJson();
}

QJsonObject JsonSerializer::deserialize_array(const QByteArray &array) const
{
    QJsonParseError error;
    auto document = QJsonDocument::fromJson(array, &error);
    if (document.isNull())
    {
        throw DeserializationError(qPrintable(QStringLiteral("json parse error: ") + error.errorString()));
    }
    return document.object();
}
