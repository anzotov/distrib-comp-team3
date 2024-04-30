#pragma once

#include "serializer.h"

#include <QByteArray>
#include <QJsonObject>

class JsonSerializer final : public Serializer<QByteArray, QJsonObject>
{
     QByteArray serialize_array(const QJsonObject &intermediate) const override final;
     QJsonObject deserialize_array(const QByteArray &array) const override final;
};