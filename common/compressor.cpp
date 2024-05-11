#include "compressor.h"

Compressor::Compressor(int compressionLevel) : m_compressionLevel(compressionLevel)
{
}

QByteArray Compressor::encode(const QByteArray &array) const
{
    return qCompress(array, m_compressionLevel);
}

QByteArray Compressor::decode(const QByteArray &array) const
{
    return qUncompress(array);
}
