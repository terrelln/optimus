#pragma once

#include <tuple>
#include <type_traits>

#include <optimus/traits.h>
#include <optimus/utility.h>

namespace optimus {

/**
 * Provides an interface that supports a subset of
 * std::tuple functionality, but has constexpr
 * support.
 */
template <typename... Types>
struct tuple;

template <>
struct tuple<> { };

template <typename Type, typename... Types>
struct tuple<Type, Types...> {
    using head_type = Type;
    using tail_type = optimus::tuple<Types...>;

    head_type head_;
    tail_type tail_;

    constexpr tuple() { }
    explicit constexpr tuple(const Type& type, const Types&... types)
            : head_(type), tail_(types...) { }
    template <
        typename UType,
        typename... UTypes,
        typename = typename ::std::enable_if<
            ::std::is_constructible<Type, UType&&>::value
        >::type
    >
    explicit constexpr tuple(UType&& arg, UTypes&&... args)
            : head_(optimus::forward<UType>(arg)),
              tail_(optimus::forward<UTypes>(args)...) { }
    template <
        typename UType,
        typename... UTypes,
        typename = typename ::std::enable_if<
            ::std::is_constructible<Type, const UType&>::value
        >::type
    >
    constexpr tuple(const optimus::tuple<UType, UTypes...>& other)
            : head_(other.head_), tail_(other.tail_) { }
    template <
        typename UType,
        typename... UTypes,
        typename = typename ::std::enable_if<
            ::std::is_constructible<Type, UType&&>::value
        >::type
    >
    constexpr tuple(optimus::tuple<UType, UTypes...>&& other)
            : head_(optimus::forward<UType>(other.head_)),
              tail_(optimus::move(other.tail_)) { }
    constexpr tuple(const tuple& other)
            : head_(other.head_), tail_(other.tail_) { }
    constexpr tuple(tuple&& other) 
            : head_(optimus::move(other.head_)), tail_(optimus::move(other.tail_)) { }
};

namespace detail {

template <typename T>
struct tuple_decay {
    using type = typename ::std::decay<T>::type;
};

template <typename T>
struct tuple_decay<::std::reference_wrapper<T>> {
    using type = T&;
};

template <typename T>
using tuple_decay_t = typename tuple_decay<T>::type;

}

template <typename... Types>
constexpr optimus::tuple<detail::tuple_decay_t<Types>...> make_tuple(Types&&... args) {
    return optimus::tuple<detail::tuple_decay_t<Types>...>{optimus::forward<Types>(args)...};
}

template <typename... Types>
constexpr optimus::tuple<Types&...> tie(Types&... args) noexcept {
    return optimus::tuple<Types&...>{args...};
}

template <typename... Types>
constexpr optimus::tuple<Types&&...> forward_as_tuple(Types&&... args) noexcept {
    return optimus::tuple<Types&&...>{optimus::forward<Types>(args)...};
}

template <typename T>
struct tuple_size;

template <typename... Types>
struct tuple_size<optimus::tuple<Types...>>
    : ::std::integral_constant<std::size_t, sizeof...(Types)> { };

template <typename T>
struct tuple_size<const T>
    : ::std::integral_constant<std::size_t, tuple_size<T>::value> { };

template <typename T>
struct tuple_size<volatile T>
    : ::std::integral_constant<std::size_t, tuple_size<T>::value> { };

template <typename T>
struct tuple_size<const volatile T>
    : ::std::integral_constant<std::size_t, tuple_size<T>::value> { };

template <std::size_t I, typename T>
struct tuple_element;

template <std::size_t I, typename T, typename... Types>
struct tuple_element<I, optimus::tuple<T, Types...>> {
    using type = typename tuple_element<I - 1, optimus::tuple<Types...>>::type;
};

template <typename T, typename... Types>
struct tuple_element<0, optimus::tuple<T, Types...>> {
    using type = T;
};

template <std::size_t I, typename T>
struct tuple_element<I, const T> {
    using type = typename ::std::add_const<optimus::tuple_element<I, T>>::type;
};

template <std::size_t I, typename T>
struct tuple_element<I, volatile T> {
    using type = typename ::std::add_volatile<optimus::tuple_element<I, T>>::type;
};

template <std::size_t I, typename T>
struct tuple_element<I, const volatile T> {
    using type = typename ::std::add_cv<optimus::tuple_element<I, T>>::type;
};

namespace detail {

template <std::size_t I, typename T>
struct get;

template <std::size_t I, typename T, typename... Types>
struct get<I, optimus::tuple<T, Types...>> {
    constexpr typename optimus::tuple_element<I, optimus::tuple<T, Types...>>::type&
    operator()(optimus::tuple<T, Types...>& tup) const {
        return get<I - 1, optimus::tuple<Types...>>{}(tup.tail_);
    }
    constexpr typename optimus::tuple_element<I, optimus::tuple<T, Types...>>::type const&
    operator()(const optimus::tuple<T, Types...>& tup) const {
        return get<I - 1, optimus::tuple<Types...>>{}(tup.tail_);
    }
    constexpr typename optimus::tuple_element<I, optimus::tuple<T, Types...>>::type&&
    operator()(optimus::tuple<T, Types...>&& tup) const {
        return get<I - 1, optimus::tuple<Types...>>{}(optimus::move(tup.tail_));
    }
};

template <typename T, typename... Types>
struct get<0, optimus::tuple<T, Types...>> {
    constexpr T& operator()(optimus::tuple<T, Types...>& tup) const {
        return tup.head_;
    }
    constexpr T const& operator()(const optimus::tuple<T, Types...>& tup) const {
        return tup.head_;
    }
    constexpr T&& operator()(optimus::tuple<T, Types...>&& tup) const {
        return optimus::move(tup.head_);
    }
};

} // namespace detail

template <std::size_t I, typename... Types>
typename std::tuple_element<I, optimus::tuple<Types...>>::type&
get(optimus::tuple<Types...>& t) {
    return optimus::detail::get<I, optimus::tuple<Types...>>{}(t);
}

template <std::size_t I, typename... Types>
typename std::tuple_element<I, optimus::tuple<Types...>>::type const&
get(optimus::tuple<Types...> const& t) {
    return optimus::detail::get<I, optimus::tuple<Types...>>{}(t);
}

template <std::size_t I, typename... Types>
typename std::tuple_element<I, optimus::tuple<Types...>>::type&&
get(optimus::tuple<Types...>&& t) {
    return optimus::detail::get<I, optimus::tuple<Types...>>{}(optimus::move(t));
}

} // namespace optimus

namespace std {

template <typename... Types>
struct tuple_size<optimus::tuple<Types...>>
    : ::std::integral_constant<std::size_t, sizeof...(Types)> { };

template <std::size_t I, typename... Types>
struct tuple_element<I, optimus::tuple<Types...>> {
    using type = typename optimus::tuple_element<I, optimus::tuple<Types...>>::type;
};

template <std::size_t I, typename... Types>
typename std::tuple_element<I, optimus::tuple<Types...>>::type&
get(optimus::tuple<Types...>& t) {
    return optimus::detail::get<I, optimus::tuple<Types...>>{}(t);
}

template <std::size_t I, typename... Types>
typename std::tuple_element<I, optimus::tuple<Types...>>::type const&
get(optimus::tuple<Types...> const& t) {
    return optimus::detail::get<I, optimus::tuple<Types...>>{}(t);
}

template <std::size_t I, typename... Types>
typename std::tuple_element<I, optimus::tuple<Types...>>::type&&
get(optimus::tuple<Types...>&& t) {
    return optimus::detail::get<I, optimus::tuple<Types...>>{}(optimus::move(t));
}

} // namespace std

namespace optimus {

template <std::size_t I, typename T>
using tuple_element_t = typename ::std::tuple_element<I, T>::type;

namespace detail {

template <typename TTuple, std::size_t... TIndices, typename UTuple, std::size_t... UIndices>
constexpr auto tuple_cat_impl(
        TTuple&& ttuple, optimus::index_sequence<TIndices...>,
        UTuple&& utuple, optimus::index_sequence<UIndices...>)
        ->  optimus::tuple<
                optimus::tuple_element_t<TIndices, TTuple>...,
                optimus::tuple_element_t<UIndices, UTuple>...
            > {
    return optimus::tuple<
        optimus::tuple_element_t<TIndices, TTuple>...,
        optimus::tuple_element_t<UIndices, UTuple>...
    >{
        ::std::get<TIndices>(optimus::forward<TTuple>(ttuple))...,
        ::std::get<UIndices>(optimus::forward<UTuple>(utuple))...
    };
}

struct tuple_cat {
    template <typename TTuple, typename UTuple>
    constexpr auto operator()(TTuple&& ttuple, UTuple&& utuple) const
        ->  decltype(tuple_cat_impl(
                optimus::forward<TTuple>(ttuple),
                optimus::make_index_sequence<::std::tuple_size<TTuple>::value>{},
                optimus::forward<UTuple>(utuple),
                optimus::make_index_sequence<::std::tuple_size<UTuple>::value>{}))
        {
        return tuple_cat_impl(
                optimus::forward<TTuple>(ttuple),
                optimus::make_index_sequence<::std::tuple_size<TTuple>::value>{},
                optimus::forward<UTuple>(utuple),
                optimus::make_index_sequence<::std::tuple_size<UTuple>::value>{});
    }
};

class foldl {
    template <typename Fn>
    struct impl {
        constexpr impl() { }
        template <
            typename... Args,
            typename = safe_forwarding_constructor_t<impl, Args...>
        >
        constexpr impl(Args&&... args)
            : fn_(optimus::forward<Args>(args)...) { }
        constexpr impl(const impl&) = default;
        constexpr impl(impl&&) = default;

        Fn fn_;

        template <typename Acc, typename Arg, typename... Args>
        constexpr auto operator()(Acc&& acc, Arg&& arg, Args&&... args) const ->
                optimus::result_of_t<impl<Fn>(
                    optimus::result_of_t<Fn(Acc&&, Arg&&)>,
                    Args&&...
                )> {
            return (*this)(
                fn_(optimus::forward<Acc>(acc), optimus::forward<Arg>(arg)), 
                optimus::forward<Args>(args)...);
        }

        template <typename Acc>
        constexpr Acc&& operator()(Acc&& acc) const {
            return optimus::forward<Acc>(acc);
        }
    };

  public:
    template <typename Fn>
    using apply = impl<Fn>;
};

}

template <typename... Tuples>
constexpr auto tuple_cat(Tuples&&... args)
        -> optimus::result_of_t<detail::foldl::apply<detail::tuple_cat>(Tuples&&...)> {
    return detail::foldl::apply<detail::tuple_cat>{}(optimus::forward<Tuples>(args)...);
}

}
