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
    Class(Class&&) = default; \
    \
    Fn fn_;

namespace optimus {

template <::std::size_t Index>
class get {
    template <typename Fn>
    struct impl {
        MAKE_BASIC_TRANSFORMER(impl)

        template <typename... Args>
        constexpr auto operator()(Args&&... args) const -> result_of_t<const Fn(decltype(::std::get<Index>(optimus::forward<Args>(args)))...)> {
            return fn_(::std::get<Index>(optimus::forward<Args>(args))...);
        }
    };

  public:
    template <typename Fn>
    using apply = impl<Fn>;
};

using fst = optimus::get<0>;
using snd = optimus::get<1>;

class identity {
  public:
    template <typename Fn>
    using apply = Fn;
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
