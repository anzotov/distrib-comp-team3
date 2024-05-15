#pragma once

#include "../common/jsonSerializer.h"
#include "../common/calcTask.h"
#include "../common/calcResult.h"
#include "../common/handshake.h"
#include "../common-tests/qtest-toString.h"

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
        const auto empty_object = QByteArray("{}");
        QVERIFY_EXCEPTION_THROWN(serializer.deserialize<SerializableTestClass>(empty_object, [&](SerializableTestClass *obj)
                                                                               { delete obj;success = true; }),
                                 DeserializationError);
        const auto invalid_array = QByteArray("{");
        QVERIFY_EXCEPTION_THROWN(serializer.deserialize<SerializableTestClass>(invalid_array, [&](SerializableTestClass *obj)
                                                                               { delete obj;success = true; }),
                                 DeserializationError);
        QVERIFY(!success);
    }

    void CalcTaskSerializationTest()
    {
        const auto serializer = JsonSerializer();
        const auto task = CalcTask{"sin(x)", {"0", "1"}, true};
        const auto array = serializer.serialize(task);
        int test_handlerCount = 0;
        serializer.deserialize<CalcTask>(array, [&](CalcTask *obj)
                                         {
                                            ++test_handlerCount;
                                            QCOMPARE(*obj, task);
                                            delete obj; });
        QCOMPARE(test_handlerCount, 1);
    }
    void CalcTaskDeserializationErrorTest()
    {
        const auto serializer = JsonSerializer();
        const auto array = "{\"class\":\"CalcTask\"}";
        int test_handlerCount = 0;
        QVERIFY_EXCEPTION_THROWN(serializer.deserialize<CalcTask>(array, [&](CalcTask *obj)
                                                                  {
                                            ++test_handlerCount;
                                            delete obj; }),
                                 DeserializationError);
        QCOMPARE(test_handlerCount, 0);
    }
    void CalcResultSerializationTest()
    {
        const auto serializer = JsonSerializer();
        const auto result = CalcResult{{"0", "1"}, true};
        const auto array = serializer.serialize(result);
        int test_handlerCount = 0;
        serializer.deserialize<CalcResult>(array, [&](CalcResult *obj)
                                           {
                                            ++test_handlerCount;
                                            QCOMPARE(*obj, result);
                                            delete obj; });
        QCOMPARE(test_handlerCount, 1);
    }
    void CalcResultDeserializationErrorTest()
    {
        const auto serializer = JsonSerializer();
        const auto array = "{\"class\":\"CalcResult\"}";
        int test_handlerCount = 0;
        QVERIFY_EXCEPTION_THROWN(serializer.deserialize<CalcResult>(array, [&](CalcResult *obj)
                                                                    {
                                            ++test_handlerCount;
                                            delete obj; }),
                                 DeserializationError);
        QCOMPARE(test_handlerCount, 0);
    }
    void HandshakeSerializationTest()
    {
        const auto serializer = JsonSerializer();
        const auto handshake = Handshake(Handshake::PeerType::TaskNode, "1000", "{12345-3-45-6}");
        const auto array = serializer.serialize(handshake);
        int test_handlerCount = 0;
        serializer.deserialize<Handshake>(array, [&](Handshake *obj)
                                          {
                                            ++test_handlerCount;
                                            QCOMPARE(*obj, handshake);
                                            delete obj; });
        QCOMPARE(test_handlerCount, 1);
    }
    void HandshakeDeserializationErrorTest()
    {
        const auto serializer = JsonSerializer();
        const auto array = "{\"class\":\"Handshake\"}";
        int test_handlerCount = 0;
        QVERIFY_EXCEPTION_THROWN(serializer.deserialize<Handshake>(array, [&](Handshake *obj)
                                                                   {
                                            ++test_handlerCount;
                                            delete obj; }),
                                 DeserializationError);
        QCOMPARE(test_handlerCount, 0);
    }
};