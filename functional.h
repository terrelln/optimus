#pragma once

#include <utility>

#include <optimus/traits.h>
#include <optimus/utility.h>

namespace optimus {

#define OPTIMUS_MAKE_FUNCTION_CONSTRUCTORS(Class) \
    constexpr Class() { } \
    constexpr Class(Class const&) = default; \
    constexpr Class(Class &&) = default;

struct id {
    OPTIMUS_MAKE_FUNCTION_CONSTRUCTORS(id)

    template <typename T>
    constexpr T&& operator()(T&& v) const {
        return optimus::forward<T>(v);
    }
};

template <typename T>
struct constant {
    constexpr constant() = delete;
    constexpr constant(const constant&) = default;
    constexpr constant(constant&&) = default;

    using result_type = T;

    template <typename... Args, typename = safe_forwarding_constructor_t<constant, Args...>>
    explicit constexpr constant(Args&&... args) : v_(optimus::forward<Args>(args)...) { }

    template <typename... Args>
    constexpr T operator()(Args&&...) const {
        return v_;
    }

    T v_;
};

template <typename Integral, Integral Value>
struct constant<::std::integral_constant<Integral, Value>> {
    OPTIMUS_MAKE_FUNCTION_CONSTRUCTORS(constant)

    using result_type = Integral;

    template <typename... Args>
    constexpr Integral operator()(Args&&...) const {
        return Value;
    }
};

#define OPTIMUS_BINARY_COMPARISON_FUNCTION_IMPL(Class, Op, Result) \
    template <typename T> \
    struct Class { \
        OPTIMUS_MAKE_FUNCTION_CONSTRUCTORS(Class) \
        \
        using result_type = Result; \
        using first_argument_type = T; \
        using second_argument_type = T; \
        \
        constexpr result_type operator()(const T& lhs, const T& rhs) const { \
            return lhs Op rhs; \
        } \
    };

#define OPTIMUS_UNARY_COMPARISON_FUNCTION_IMPL(Class, Op, Result) \
    template <typename T> \
    struct Class { \
        OPTIMUS_MAKE_FUNCTION_CONSTRUCTORS(Class) \
        \
        using result_type = Result; \
        using argument_type = T; \
        \
        constexpr result_type operator()(const T& arg) const { \
            return Op(arg); \
        } \
    };

#define OPTIMUS_UNARY_COMPARISON_FUNCTION(Class, Op) \
    OPTIMUS_UNARY_COMPARISON_FUNCTION_IMPL(Class, Op, T)
#define OPTIMUS_BINARY_COMPARISON_FUNCTION(Class, Op) \
    OPTIMUS_BINARY_COMPARISON_FUNCTION_IMPL(Class, Op, T)

#define OPTIMUS_UNARY_COMPARISON_PREDICATE(Class, Op) \
    OPTIMUS_UNARY_COMPARISON_FUNCTION_IMPL(Class, Op, bool)
#define OPTIMUS_BINARY_COMPARISON_PREDICATE(Class, Op) \
    OPTIMUS_BINARY_COMPARISON_FUNCTION_IMPL(Class, Op, bool)

OPTIMUS_BINARY_COMPARISON_FUNCTION(plus, +)
OPTIMUS_BINARY_COMPARISON_FUNCTION(minus, -)
OPTIMUS_BINARY_COMPARISON_FUNCTION(multiplies, *)
OPTIMUS_BINARY_COMPARISON_FUNCTION(divides, /)
OPTIMUS_BINARY_COMPARISON_FUNCTION(modulus, %)
OPTIMUS_UNARY_COMPARISON_FUNCTION(negate, -)

OPTIMUS_BINARY_COMPARISON_PREDICATE(equal_to, ==)
OPTIMUS_BINARY_COMPARISON_PREDICATE(not_equal_to, !=)
OPTIMUS_BINARY_COMPARISON_PREDICATE(greater, >)
OPTIMUS_BINARY_COMPARISON_PREDICATE(less, <)
OPTIMUS_BINARY_COMPARISON_PREDICATE(greater_equal, >=)
OPTIMUS_BINARY_COMPARISON_PREDICATE(less_equal, <=)

OPTIMUS_BINARY_COMPARISON_PREDICATE(logical_and, &&)
OPTIMUS_BINARY_COMPARISON_PREDICATE(logical_or, ||)
OPTIMUS_UNARY_COMPARISON_PREDICATE(logical_not, !)

OPTIMUS_BINARY_COMPARISON_FUNCTION(bit_and, &)
OPTIMUS_BINARY_COMPARISON_FUNCTION(bit_or, |)
OPTIMUS_BINARY_COMPARISON_FUNCTION(bit_xor, ^)

#undef OPTIMUS_UNARY_COMPARISON_FUNCTION
#undef OPTIMUS_BINARY_COMPARISON_FUNCTION
#undef OPTIMUS_UNARY_COMPARISON_PREDICATE
#undef OPTIMUS_BINARY_COMPARISON_PREDICATE
#undef OPTIMUS_BINARY_COMPARISON_FUNCTION_IMPL
#undef OPTIMUS_UNARY_COMPARISON_FUNCTION_IMPL

template <template <typename...> class BinOp, typename TArg, typename... TArgs>
struct foldr1 {
  using type = typename BinOp<TArg, typename foldr1<BinOp, TArgs...>::type>::type;
};

template <template <typename...> class BinOp, typename TArg>
struct foldr1<BinOp, TArg> {
    using type = TArg;
};

template <typename Acc, template <typename...> class BinOp, typename... TArgs>
using foldr = foldr1<BinOp, TArgs..., Acc>;

template <template <typename...> class BinOp, typename Acc, typename... TArgs>
struct foldl;

template <template <typename...> class BinOp, typename Acc, typename TArg, typename... TArgs>
struct foldl<BinOp, Acc, TArg, TArgs...> {
    using type = typename foldl<BinOp, typename BinOp<Acc, TArg>::type, TArgs...>::type;
};

template <template <typename...> class BinOp, typename Acc>
struct foldl<BinOp, Acc> {
    using type = Acc;
};


template <template <typename...> class BinOp, typename Acc, typename... TArgs>
using foldl1 = foldl<BinOp, Acc, TArgs...>;

#undef OPTIMUS_MAKE_FUNCTION_CONSTRUCTORS
}
