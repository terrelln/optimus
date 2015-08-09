#pragma once

#include <tuple>
#include <type_traits>

namespace optimus {

// Until C++14 forward is not constexpr, delete once C++14 is supported
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

// Until C++14 move is not constexpr, delete once C++14 is supported
template <typename T>
constexpr typename std::remove_reference<T>::type&& move(T&& t) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}

// Remove once C++14 is supported
template <typename T, T... Ints>
struct integer_sequence {
    using value_type = T;

    static constexpr std::size_t size() noexcept {
        return sizeof...(Ints);
    }
};

namespace detail {

// Concat 2 integer sequences of the same type
template <typename Sequence1, typename Sequence2>
struct concat;

template <typename T, T... Ints1, T... Ints2>
struct concat<integer_sequence<T, Ints1...>, integer_sequence<T, Ints2...>> {
    using type = integer_sequence<T, Ints1..., Ints2...>;
};

// Find the midpoint of 2 sequences
template <typename T>
constexpr T middle(T low, T high) {
    return (high + low) / 2;
}

// Split the list in half + concat
// The weird hack with the bool is so the compiler dones't think we have
// infinite recursion
template <typename T, T Low, T High, bool B>
struct make_integer_sequence_impl {
    using type = typename concat<
        typename make_integer_sequence_impl<T, Low, middle(Low, High), false>::type,
        typename make_integer_sequence_impl<T, middle(Low, High), High, middle(Low, High) + 1 == High>::type
    >::type;
};

// Handles the case where Low + 1 == High.
template <typename T, T Low, T High>
struct make_integer_sequence_impl<T, Low, High, true> {
    using type = integer_sequence<T, Low>;
};

template <typename T, T Same>
struct make_integer_sequence_impl<T, Same, Same, false> {
    using type = integer_sequence<T>;
};

}

template <std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

template <typename T, T N>
using make_integer_sequence = typename detail::make_integer_sequence_impl<T, 0, N, N == 1>::type;

template <std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template <typename... Args>
using index_sequence_for = make_index_sequence<sizeof...(Args)>;


// Remove once C++11 is here
template <typename Func, typename Tup, std::size_t... Indices>
constexpr auto apply_impl(Func&& func, Tup&& tup, optimus::index_sequence<Indices...>) ->
        decltype(
           optimus::forward<Func>(func)(::std::get<Indices>(optimus::forward<Tup>(tup))...)) {
    return optimus::forward<Func>(func)(::std::get<Indices>(optimus::forward<Tup>(tup))...);
}

// I love writing things twice in C++11.
template <typename Func, typename Tup>
constexpr auto apply(Func&& func, Tup&& tup) ->
        decltype(
           apply_impl(
            optimus::forward<Func>(func),
            optimus::forward<Tup>(tup),
            optimus::make_index_sequence<std::tuple_size<typename std::decay<Tup>::type>::value>{})) {
    return apply_impl(
            optimus::forward<Func>(func),
            optimus::forward<Tup>(tup),
            optimus::make_index_sequence<std::tuple_size<typename std::decay<Tup>::type>::value>{});
}

}
