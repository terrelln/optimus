#include <algorithm>
#include <functional>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include <optimus/functional.h>
#include <optimus/transformers.h>

#define EXPECT_SAME_TYPE(T, U) \
    EXPECT_TRUE((std::is_same<T, U>::value))

#define EXPECT_SAME_TYPE_AS(T, Value) \
    EXPECT_SAME_TYPE(T, decltype(Value))

template <typename T>
class transformer_test : public ::testing::Test { };

typedef ::testing::Types<
    optimus::fst,
    optimus::snd,
    optimus::get<2>,
    optimus::id,
    optimus::flip,
    optimus::variadic<optimus::id, optimus::constant<std::integral_constant<int, 42>>>,
    optimus::after<optimus::logical_not>,
    optimus::before<optimus::negate>,
    optimus::compose<optimus::fst, optimus::snd>
> transformer_types;


TYPED_TEST_CASE(transformer_test, transformer_types);

TYPED_TEST(transformer_test, constructors) {
    typename TypeParam::template apply<optimus::constant<int>> t{int(5)};
    typename TypeParam::template apply<optimus::constant<int>> t2{t};
    typename TypeParam::template apply<optimus::constant<int>> t3{std::move(t2)};
}

TYPED_TEST(transformer_test, constexpr_constructors) {
    constexpr typename TypeParam::template apply<optimus::constant<int>> t{5};
    constexpr typename TypeParam::template apply<optimus::constant<int>> t2{t};
}

struct expecter {
    expecter(bool copyable, bool movable) : copyable(copyable), movable(movable) { }
    expecter(const expecter& o) : copyable(o.copyable), movable(o.movable) {
        EXPECT_TRUE(copyable);
    }
    expecter(expecter&& o) : copyable(o.copyable), movable(o.movable) {
        EXPECT_TRUE(movable);
    }

    bool copyable;
    bool movable;

    bool operator <(const expecter& other) const {
        if (!copyable && other.copyable) {
            return true;
        } else if (!movable && other.movable) {
            return true;
        } else {
            return false;
        }
    }
};

struct A { };
struct B { };

TEST(get, forwarding) {
    auto fn = optimus::fst::apply<optimus::id>{};
    auto t = std::make_tuple(A{}, B{});
    const auto u = std::make_tuple(A{}, B{});
    EXPECT_SAME_TYPE_AS(A&&, fn(std::make_tuple(A{}, B{})));
    EXPECT_SAME_TYPE_AS(A&, fn(t));
    EXPECT_SAME_TYPE_AS(A&&, fn(std::move(t)));
    EXPECT_SAME_TYPE_AS(const A&, fn(u));

    auto e1 = fn(std::make_tuple(expecter(false, true), 0, false, true, 9));
    auto e2 = fn(std::make_tuple(expecter(false, true), true));
    auto e3 = expecter{true, false};
    auto v = std::make_tuple(e3, 0);
    auto e4 = fn(v);
}

TEST(get, works) {
    auto fn1 = optimus::fst::apply<optimus::less<int>>{};
    auto fn2 = optimus::snd::apply<optimus::less<int>>{};

    std::vector<std::pair<int, int>> v1 = {{0, 3}, {2, 2}, {1, 0}, {3, 1}};
    decltype(v1) v2, v3, v4;
    v4 = v3 = v2 = v1;

    std::sort(v1.begin(), v1.end(), fn1);
    std::sort(v2.begin(), v2.end(),
            [](const std::pair<int, int>& x, const std::pair<int, int>& y) {
                return x.first < y.first;
            });
    EXPECT_EQ(v2, v1);

    std::sort(v3.begin(), v3.end(), fn2);
    std::sort(v4.begin(), v4.end(),
            [](const std::pair<int, int>& x, const std::pair<int, int>& y) {
                return x.second < y.second;
            });
    EXPECT_EQ(v4, v3);
}

// Doesn't work on clang (boo tuple, boo).
/*
TEST(get, constexpr) {
    constexpr auto getter = optimus::fst::apply<optimus::id>{};
    constexpr std::tuple<int, int> t{0, 1};
    EXPECT_EQ(0, (std::integral_constant<int, getter(t)>::value));
}
*/

TEST(id, constexpr_operator) {
    constexpr auto less = optimus::id::apply<optimus::less<int>>{};
    EXPECT_EQ(true, (std::integral_constant<bool, less(5, 6)>::value));
    EXPECT_EQ(false, (std::integral_constant<bool, less(5, 5)>::value));
}

TEST(flip, works) {
    auto f = optimus::flip::apply<optimus::less<int>>{};
    auto g = optimus::greater<int>{};

    EXPECT_EQ(g(5, 5), f(5, 5));
    EXPECT_EQ(g(4, 5), f(4, 5));
    EXPECT_EQ(g(6, 5), f(6, 5));
}

TEST(flip, constexpr_operator) {
    constexpr auto less = optimus::flip::apply<optimus::greater<int>>{};
    EXPECT_EQ(true, (std::integral_constant<bool, less(5, 6)>::value));
    EXPECT_EQ(false, (std::integral_constant<bool, less(5, 5)>::value));
}

TEST(variadic, works) {
    using fst_low = optimus::fst::apply<optimus::id>;
    using snd_low = optimus::snd::apply<optimus::id>;
    auto f = optimus::variadic<fst_low, snd_low>::apply<optimus::less<int>>{};
    auto zo = std::make_tuple(0, 1);
    auto oz = std::make_tuple(1, 0);
    EXPECT_TRUE(f(zo, zo));
    EXPECT_FALSE(f(zo, oz));
    EXPECT_FALSE(f(oz, zo));
    EXPECT_FALSE(f(oz, oz));
}

TEST(variadic, forwarding) {
    using fst_low = optimus::fst::apply<optimus::id>;
    using snd_low = optimus::snd::apply<optimus::id>;
    auto f = optimus::variadic<fst_low, snd_low>::apply<optimus::less<expecter>>{};

    auto e = expecter{true, false};
    auto p = std::pair<expecter, expecter>(
            std::piecewise_construct,
            std::forward_as_tuple(false, false),
            std::forward_as_tuple(false, false));
    EXPECT_TRUE(f(std::make_tuple(expecter{false, true}, expecter{false, true}),
            std::make_tuple(expecter{false, true}, e)));
    EXPECT_FALSE(f(p, p));
}

TEST(variadic, constexpr_operator) {
    constexpr auto f = optimus::variadic<optimus::id>::apply<optimus::id>{};
    EXPECT_EQ(true, (std::integral_constant<bool, f(true)>::value));
    EXPECT_EQ(false, (std::integral_constant<bool, f(false)>::value));
}

TEST(after, works) {
    auto fn = optimus::after<optimus::logical_not>::apply<optimus::logical_or<bool>>{};
    EXPECT_EQ(true, fn(false, false));
    EXPECT_EQ(false, fn(false, true));
    EXPECT_EQ(false, fn(true, false));
    EXPECT_EQ(false, fn(true, true));
}

TEST(after, constexpr_operator) {
    constexpr auto fn = optimus::after<optimus::logical_not>::apply<optimus::logical_or<bool>>{};
    EXPECT_EQ(true, (std::integral_constant<bool, fn(false, false)>::value));
    EXPECT_EQ(false, (std::integral_constant<bool, fn(false, true)>::value));
    EXPECT_EQ(false, (std::integral_constant<bool, fn(true, false)>::value));
    EXPECT_EQ(false, (std::integral_constant<bool, fn(true, true)>::value));
}

TEST(before, works) {
    auto fn = optimus::before<optimus::logical_not>::apply<optimus::logical_or<bool>>{};
    EXPECT_EQ(true, fn(false, false));
    EXPECT_EQ(true, fn(false, true));
    EXPECT_EQ(true, fn(true, false));
    EXPECT_EQ(false, fn(true, true));
}

TEST(before, constexpr_operator) {
    constexpr auto fn = optimus::before<optimus::logical_not>::apply<optimus::logical_or<bool>>{};
    EXPECT_EQ(true, (std::integral_constant<bool, fn(false, false)>::value));
    EXPECT_EQ(true, (std::integral_constant<bool, fn(false, true)>::value));
    EXPECT_EQ(true, (std::integral_constant<bool, fn(true, false)>::value));
    EXPECT_EQ(false, (std::integral_constant<bool, fn(true, true)>::value));
}

TEST(compose, works) {
    using t1 = optimus::compose<
            optimus::after<optimus::logical_not>,
            optimus::flip,
            optimus::before<optimus::logical_not>
        >;
    using t2 = optimus::after<optimus::logical_not>::apply<
            optimus::flip::apply<
                optimus::before<optimus::logical_not>::apply<
                    optimus::id
                >
            >
        >;
    EXPECT_SAME_TYPE(t1::apply<optimus::id>, t2);

    using t3 = optimus::compose<optimus::id>::apply<optimus::id>;
    using t4 = optimus::id::apply<optimus::id>;
    EXPECT_SAME_TYPE(t3, t4);

    EXPECT_EQ(false, (std::integral_constant<bool, t1::apply<optimus::less<bool>>{}(false, true)>::value));
}

#undef EXPECT_SAME_TYPE
#undef EXPECT_SAME_TYPE_AS
