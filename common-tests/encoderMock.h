#pragma once

#include "../common/encoderBase.h"

#include <QByteArray>
#include <functional>

struct EncoderMock final : EncoderBase<QByteArray>
{
    QByteArray encode(const QByteArray &array) const override final
    {
        ++m_encodeCount;
        return m_encode(this, array);
    }
    QByteArray decode(const QByteArray &array) const override final
    {
        ++m_decodeCount;
        return m_decode(this, array);
    }

    std::function<QByteArray(const EncoderMock *, const QByteArray &)> m_encode;
    mutable int m_encodeCount = 0;
    std::function<QByteArray(const EncoderMock *, const QByteArray &)> m_decode;
    mutable int m_decodeCount = 0;
};
