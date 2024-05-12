#pragma once

#include "../common/calcTask.h"
#include "../common/calcResult.h"
#include "../common/handshake.h"

#include <QTest>

inline char *toString(const CalcTask &obj)
{
    return QTest::toString(obj.toQString());
}
inline char *toString(const CalcResult &obj)
{
    return QTest::toString(obj.toQString());
}
inline char *toString(const Handshake &obj)
{
    return QTest::toString(obj.toQString());
}
