#include <memory>

template <class Type, class Functor>
class SafeHandler final
{
public:
    SafeHandler(Functor handler) : m_handler(handler) {}

    void operator()(Type *obj)
    {
        m_handler(std::unique_ptr<Type>(obj));
    }

private:
    Functor m_handler;
};

template <class Type, class Functor>
static SafeHandler<Type, Functor> makeSafeHandler(Functor handler)
{
    return SafeHandler<Type, Functor>(handler);
}
