#pragma once

#include "../common/jsonSerializer.h"
#include "../common/calcTask.h"
#include "../common/calcResult.h"

#include <QObject>
#include <QJsonObject>
#include <QtTest>
#include <stdexcept>

class CommonTests : public QObject
{
    Q_OBJECT

private slots:
    void JsonSerializerTest()
    {
        struct SerializableTestClass
        {
            QJsonObject serialize() const
            {
                return QJsonObject({{"class", "SerializableTestClass"}, {"value", "Test"}});
            }
            static bool deserialize(const QJsonObject &input, std::function<void(SerializableTestClass *)> handler)
            {
                if (input["class"] == "SerializableTestClass" && input["value"] == "Test")
                {
                    handler(new SerializableTestClass());
                    return true;
                }
                return false;
            }
        };
        bool success = false;
        const auto serializer = JsonSerializer();
        const auto array = serializer.serialize(SerializableTestClass());
        serializer.deserialize<SerializableTestClass>(array, [&](SerializableTestClass *obj)
                                                      { delete obj;success = true; });
        QVERIFY(success);
        success = false;
        const auto invalid_array = QByteArray("{}");
        QVERIFY_EXCEPTION_THROWN(serializer.deserialize<SerializableTestClass>(invalid_array, [&](SerializableTestClass *obj)
                                                                               { delete obj;success = true; }),
                                 std::logic_error);
        QVERIFY(!success);
    }

    void CalcTaskSerializationTest()
    {
        bool success = false;
        const auto serializer = JsonSerializer();
        const auto task = CalcTask{"sin(x)", {"0", "1"}, true};
        const auto array = serializer.serialize(task);
        serializer.deserialize<CalcTask>(array, [&](CalcTask *obj)
                                         {
                                            if (obj->function == task.function && obj->data == task.data && obj->isMain==task.isMain)
                                            {
                                                success = true;
                                            };
                                            delete obj; });
        QVERIFY(success);
    }
    void CalcResultSerializationTest()
    {
        bool success = false;
        const auto serializer = JsonSerializer();
        const auto task = CalcResult{{"0", "1"}, true};
        const auto array = serializer.serialize(task);
        serializer.deserialize<CalcResult>(array, [&](CalcResult *obj)
                                           {
                                            if (obj->data == task.data && obj->isMain==task.isMain)
                                            {
                                                success = true;
                                            };
                                            delete obj; });
        QVERIFY(success);
    }
};