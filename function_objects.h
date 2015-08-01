#pragma once

#include <utility>

#include "traits.h"

namespace optimus {

struct id {
    template <typename T>
    constexpr T&& operator()(T&& v) const {
        return ::std::forward<T>(v);
    }
};

template <typename T>
struct constant {
    template <typename... Args, typename = safe_forwarding_constructor<constant, Args...>>
    explicit constexpr constant(Args&&... args) : v_(std::forward<Args>(args)...) { }

    template <typename... Args>
    constexpr T operator()(Args&&...) const {
        return v_;
    }

    T v_;
};

template <typename Integral, Integral Value>
struct constant<::std::integral_constant<Integral, Value>> {
    template <typename... Args>
    constexpr Integral operator()(Args&&...) const {
        return Value;
    }
};

}
