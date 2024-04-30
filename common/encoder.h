#pragma once

template <class ArrayType>
struct Encoder
{
    virtual ~Encoder() = default;
    virtual ArrayType encode(const ArrayType &array) const = 0;
    virtual ArrayType decode(const ArrayType &array) const = 0;
};