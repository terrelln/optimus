#pragma once

#include <type_traits>

namespace optimus {

// Remove once C++ 14 is here.
template <typename T>
using result_of_t = typename ::std::result_of<T>::type;

template <typename, typename...>
struct safe_forwarding_constructor : ::std::true_type { };

template <typename Class, typename T>
struct safe_forwarding_constructor<Class, T> :
    ::std::integral_constant<
        bool,
        !std::is_base_of<
            Class,
            typename ::std::decay<T>::type
        >::value
    >
{ };

template <typename Class, typename... Args>
using safe_forwarding_constructor_t = typename ::std::enable_if<
    safe_forwarding_constructor<Class, Args...>::value
>::type;

}
