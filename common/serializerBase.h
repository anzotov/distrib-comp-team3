#pragma once

#include <functional>
#include <stdexcept>

struct DeserializationError final : std::logic_error
{
    DeserializationError(const std::string &what_arg) : std::logic_error(what_arg) {}
    DeserializationError(const char *what_arg) : std::logic_error(what_arg) {}
    DeserializationError(const DeserializationError &other) : std::logic_error(other) {}
};

template <class ArrayType, class IntermediateType>
struct SerializerBase
{
    virtual ~SerializerBase() = default;
    
    template <class T>
    ArrayType serialize(const T &object) const
    {
        return serialize_array(object.serialize());
    }

    template <class T, class... Ts>
    void deserialize(const ArrayType &array, std::function<void(T *)> handler, std::function<void(Ts *)>... handlers) const
    {
        auto intermediate = deserialize_array(array);
        deserialize_inner<T, Ts...>(intermediate, handler, handlers...);
    }

protected:
    virtual ArrayType serialize_array(const IntermediateType &intermediate) const = 0;
    virtual IntermediateType deserialize_array(const ArrayType &array) const = 0;

private:
    template <class T, class... Ts>
    void deserialize_inner(const IntermediateType &intermediate, std::function<void(T *)> handler, std::function<void(Ts *)>... handlers) const
    {
        if (T::deserialize(intermediate, handler))
        {
            return;
        }
        if constexpr (sizeof...(Ts))
        {
            deserialize_inner<Ts...>(intermediate, handlers...);
        }
        else
        {
            throw DeserializationError("Unknown object");
        }
    }
};