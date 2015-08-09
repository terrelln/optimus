#pragma once

#include <tuple>
#include <type_traits>

#include <optimus/functional.h>
#include <optimus/traits.h>
#include <optimus/utility.h>

#define MAKE_BASIC_TRANSFORMER(Class) \
    constexpr Class() { } \
    \
    template < \
        typename... Args, \
        typename = safe_forwarding_constructor_t<Class, Args...> \
    > \
    explicit constexpr Class(Args&&... args) : \
            fn_(optimus::forward<Args>(args)...) { } \
    \
    constexpr Class(const Class&) = default; \
    constexpr Class(Class&&) = default; \
    \
    Fn fn_;

namespace optimus {

template <::std::size_t Index>
class get {
    template <typename Fn>
    struct transformer_impl {
        MAKE_BASIC_TRANSFORMER(transformer_impl)

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const -> result_of_t<const Fn(decltype(::std::get<Index>(optimus::forward<Args>(args)))...)> {
            return fn_(::std::get<Index>(optimus::forward<Args>(args))...);
        }
    };

  public:
    template <typename Fn>
    using apply = transformer_impl<Fn>;

    constexpr get() { }
    constexpr get(const get&) = default;
    get(get&&) = default;

    template <typename Arg>
    constexpr auto operator()(Arg&& arg) const -> decltype(::std::get<Index>(optimus::forward<Arg>(arg))) {
        return ::std::get<Index>(optimus::forward<Arg>(arg));
    }
};

using fst = optimus::get<0>;
using snd = optimus::get<1>;

class id {
  public:
    template <typename Fn>
    using apply = Fn;

    constexpr id() { }
    constexpr id(const id&) = default;
    id(id&&) = default;

    template <typename Arg>
    constexpr Arg&& operator()(Arg&& arg) const  {
        return optimus::forward<Arg>(arg);
    }
};

class flip {
    template <typename Fn>
    struct impl { 
        MAKE_BASIC_TRANSFORMER(impl)

        template <typename Lhs, typename Rhs>
        constexpr auto operator()(Lhs&& lhs, Rhs&& rhs) const -> result_of_t<const Fn(Rhs&&, Lhs&&)> {
            return fn_(optimus::forward<Rhs>(rhs), optimus::forward<Lhs>(lhs));
        }
    };

  public:
    template <typename Fn>
    using apply = impl<Fn>;
};

// Will allow arguments to all of the Fns in the future.
// Would like to use a tuple to do this.
template <typename... Fns>
class variadic {
    template <typename Fn>
    struct impl {
        MAKE_BASIC_TRANSFORMER(impl)

        template <typename... Args, typename = typename ::std::enable_if<sizeof...(Fns) == sizeof...(Args)>::type>
        constexpr auto operator()(Args&&... args) const -> result_of_t<const Fn(result_of_t<const Fns(Args&&)>...)> {
            return this->fn_(Fns{}(optimus::forward<Args>(args))...);
        }
    };

  public:
    template <typename Fn>
    using apply = impl<Fn>;
};

template <typename Key>
class at {
    template <typename Fn>
    struct impl {
        impl() { }

        template <
            typename KeyArg,
            typename... Args,
            typename = safe_forwarding_constructor_t<impl, KeyArg, Args...>
        >
        explicit constexpr impl(KeyArg&& key_arg, Args&&... args)
                : key_(optimus::forward<KeyArg>(key_arg)),
                  fn_(optimus::forward<Args>(args)...) { }

        constexpr impl(const impl&) = default;
        impl(impl&&) = default;

        Key key_;
        Fn fn_;

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const -> result_of_t<const Fn(decltype(optimus::forward<Args>(args).at(key_))...)> {
            return this->fn_(optimus::forward<Args>(args).at(key_)...);
        }
    };

  public:
    template <typename Fn>
    using apply = impl<Fn>;

    template <
        typename... Args,
        typename = safe_forwarding_constructor_t<at, Args...>
    >
    constexpr at(Args&&... args)
            : key_(optimus::forward<Args>(args)...)  { }
    constexpr at(const at&) = default;
    at(at&&) = default;

    Key key_;

    template <typename Arg>
    constexpr auto operator()(Arg&& arg) const -> decltype(optimus::forward<Arg>(arg).at(key_)) {
        return optimus::forward<Arg>(arg).at(key_);
    }
};

template <template <typename U, U> class Constant, typename T, T value>
class at<Constant<T, value>> {
    template <typename Fn>
    struct impl {
        MAKE_BASIC_TRANSFORMER(impl)

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const -> result_of_t<const Fn(decltype(std::forward<Args>(args).at(value))...)> {
            return this->fn_(std::forward<Args>(args).at(value)...);
        }
    };

  public:
    template <typename Fn>
    using apply = impl<Fn>;

    constexpr at() { }
    constexpr at(const at&) = default;
    at(at&&) = default;

    T key_;

    template <typename Arg>
    constexpr auto operator()(Arg&& arg) const -> decltype(optimus::forward<Arg>(arg).at(value)) {
        return optimus::forward<Arg>(arg).at(value);
    }
};

template <std::size_t Index>
using at_index = at<std::integral_constant<std::size_t, Index>>;

template <template <typename...> class Function>
class after {
    template <typename Fn>
    struct impl {
        MAKE_BASIC_TRANSFORMER(impl)

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const -> result_of_t<Function<result_of_t<const Fn(Args&&...)>>(result_of_t<const Fn(Args&&...)>)> {
            return Function<result_of_t<const Fn(Args&&...)>>{}(this->fn_(optimus::forward<Args>(args)...));
        }
    };

  public:
    template <typename Fn>
    using apply = impl<Fn>;
};

template <template <typename...> class Function>
class before {
    template <typename Fn>
    struct impl {
        MAKE_BASIC_TRANSFORMER(impl)

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const -> result_of_t<const Fn(result_of_t<Function<Args>(Args&&)>...)> {
            return this->fn_(Function<Args>{}(optimus::forward<Args>(args))...);
        }
    };

  public:
    template <typename Fn>
    using apply = impl<Fn>;
};

template <typename... Transforms>
class compose {
    template <typename Transform, typename Fn>
    struct composer {
        using type = typename Transform::template apply<Fn>;
    };

  public:
    template <typename Fn>
    using apply = typename foldr<Fn, composer, Transforms...>::type;
};

}

#undef MAKE_BASIC_TRANSFORMER
