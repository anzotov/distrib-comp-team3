#pragma once

#include "../common/jsonSerializer.h"
#include "../common/calcTask.h"
#include "../common/calcResult.h"
#include "../common/handshake.h"

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
        const auto serializer = JsonSerializer();
        const auto task = CalcTask{"sin(x)", {"0", "1"}, true};
        const auto array = serializer.serialize(task);
        serializer.deserialize<CalcTask>(array, [&](CalcTask *obj)
                                         {
                                            QCOMPARE(*obj, task);
                                            delete obj; });
    }
    void CalcResultSerializationTest()
    {
        const auto serializer = JsonSerializer();
        const auto result = CalcResult{{"0", "1"}, true};
        const auto array = serializer.serialize(result);
        serializer.deserialize<CalcResult>(array, [&](CalcResult *obj)
                                           {
                                            QCOMPARE(*obj, result);
                                            delete obj; });
    }
    void HandshakeSerializationTest()
    {
        const auto serializer = JsonSerializer();
        const auto handshake = Handshake(Handshake::PeerType::TaskNode, "1000", "{12345-3-45-6}");
        const auto array = serializer.serialize(handshake);
        serializer.deserialize<Handshake>(array, [&](Handshake *obj)
                                           {
                                            QCOMPARE(*obj, handshake);
                                            delete obj; });
    }
};