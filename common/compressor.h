#pragma once

#include "encoderBase.h"

#include <QByteArray>

class Compressor final : public EncoderBase<QByteArray>
{
public:
    Compressor(int compressionLevel = -1);
    QByteArray encode(const QByteArray &array) const override final;
    QByteArray decode(const QByteArray &array) const override final;

private:
    int m_compressionLevel = -1;
};