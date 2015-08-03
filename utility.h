#pragma once

#include <type_traits>

namespace optimus {

// Until C++14 forward is not constexpr, delete once C++11 is supported
template <typename T>
constexpr T&& forward(typename std::remove_reference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}

template <typename T>
constexpr T&& forward(typename std::remove_reference<T>::type&& t) noexcept {
    static_assert(!std::is_lvalue_reference<T>::value,
        "Cannot forward a rvalue as a lvalue.");
    return static_cast<T&&>(t);
}

}
