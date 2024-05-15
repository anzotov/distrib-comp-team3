#pragma once

template <class ArrayType>
struct EncoderBase
{
    virtual ~EncoderBase() = default;
    virtual ArrayType encode(const ArrayType &array) const = 0;
    virtual ArrayType decode(const ArrayType &array) const = 0;
};