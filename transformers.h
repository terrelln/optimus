#pragma once

#include <tuple>
#include <type_traits>

namespace optimus {

namespace detail {

template <typename Fn>
struct base_transformer {
    template <typename... Args, typename = safe_forwarding_constructor_t<get, Args...>>
    explicit constexpr base_transformer(Args&&... args) : fn_(::std::forward<Args>(args)...) { }

    constexpr base_transformer(const base_transformer&) = default;
    constexpr base_transformer(base_transformer&&) = default;

    Fn fn_;
};

}

template <::std::size_t Index, typename Fn>
struct get : base_transformer<Fn> {
    using base_transformer<Fn>::base_transformer;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> result_of_t<const Fn(decltype(std::get<Index>(std::forward<Args>(args)))...)> {
        return fn_(::std::forward<decltype(::std::get<Index>(args))>(::std::get<Index>(std::forward<Args>(args)))...);
    }

    Fn fn_;
};

template <typename Fn>
using fst = get<0, Fn>;
template <typename Fn>
using snd = get<1, Fn>;

template <typename Fn>
struct identity : base_transformer<Fn> {
    using base_transformer<Fn>::base_transformer;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> result_of_t<const Fn(Args&&...)> {
        return fn_(::std::forward<Args>(args)...);
    }
};

template <typename Fn>
struct flip : base_transformer<Fn> {
    using base_transformer<Fn>::base_transformer;

    template <typename Lhs, typename Rhs>
    conxtexpr auto operator()(Lhs&& lhs, Rhs&& rhs) const -> result_of_t<const Fn(Rhs&&, Lhs&&)> {
        return fn_(::std::forward<Rhs>(rhs), ::std::forward<Lhs>(lhs));
    }
};

// Will allow arguments to all of the Fns in the future.
template <typename Fn, typename... Fns>
struct variadic : detail::base_transformer<Fn> {
    using detail::base_transformer<Fn>::base_transformer;

    template <typename... Args, typename = typename std::enable_if<sizeof...(Fns) == sizeof...(Args)>::type>
    constexpr auto operator()(Args&&... args) const -> result_of_t<const Fn(result_of_t<const Fns(Args&&)>...)> {
        return this->fn_(::std::forward<result_of_t<Fns(Args&&)>>(Fns{}(::std::forward<Args>(args)))...);
    }
};

}
