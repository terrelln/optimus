#include <functional>
#include <limits>
#include <random>
#include <type_traits>

#include <gtest/gtest.h>

#include <optimus/functional.h>

#define EXPECT_SAME_TYPE(T, U) \
    EXPECT_TRUE((std::is_same<T, U>::value))

#define EXPECT_SAME_TYPE_AS(T, Value) \
    EXPECT_SAME_TYPE(T, decltype(Value))

struct inplace {
    inplace() { }
    inplace(const inplace&) {
        EXPECT_TRUE(false);
    }
    inplace(inplace&&) {
        EXPECT_TRUE(false); 
    }
};

struct A {
    constexpr A(int value) : value(value) { }

    int value;

    bool operator ==(const A& other) const {
        return value == other.value;
    }

    bool operator !=(const A& other) const {
        return value != other.value;
    }

    constexpr int foo() const { return value; }
};

TEST(id, forwarding) {
    optimus::id id{};
    const inplace const_a{};
    inplace a{};
    EXPECT_SAME_TYPE_AS(const inplace&, id(const_a));
    EXPECT_SAME_TYPE_AS(inplace&, id(a));
    EXPECT_SAME_TYPE_AS(inplace&&, id(inplace{}));
    EXPECT_SAME_TYPE_AS(inplace&&, id(std::move(a)));
}

TEST(id, constexpr) {
    constexpr optimus::id id{};
    using t = std::integral_constant<int, id(5)>;
    EXPECT_EQ(5, (std::integral_constant<int, id(5)>::value));
    constexpr bool b = true;
    EXPECT_EQ(true, (std::integral_constant<int, id(true)>::value));

}

TEST(constant, regular) {
    optimus::constant<A> c{5};
    const inplace ci{};
    inplace i{};
    EXPECT_EQ(A{5}, c(inplace{}, ci, i, std::move(i)));
    EXPECT_EQ(A{5}, c(inplace{}, A{6}));
}

TEST(constant, integral_constant) {
    optimus::constant<std::integral_constant<int, 42>> c;
    const inplace ci{};
    inplace i{};
    EXPECT_EQ(42, c(inplace{}, ci, i, std::move(i), 41, 8));
}

TEST(constant, regular_constexpr) {
    constexpr optimus::constant<A> c{4};
    EXPECT_EQ(4, (std::integral_constant<int, c(4, 5, A{9}).value>::value));
}

TEST(constant, integral_constant_constexpr) {
    constexpr optimus::constant<std::integral_constant<char, 42>> c;
    EXPECT_EQ(42, (std::integral_constant<char, c(4, 5, A{9})>::value));
}

template <typename, typename>
struct add;

template <typename Integral, Integral A, Integral B>
struct add<std::integral_constant<Integral, A>, std::integral_constant<Integral, B>> {
    using type = std::integral_constant<Integral, A + B>;
};

template <typename, typename>
struct store_first;

template <typename Integral1, typename Integral2, Integral1 x, Integral2 y>
struct store_first<std::integral_constant<Integral1, x>, std::integral_constant<Integral2, y>> {
    using type = std::integral_constant<int, x>;
};

template <typename, typename>
struct store_second;

template <typename Integral1, typename Integral2, Integral1 x, Integral2 y>
struct store_second<std::integral_constant<Integral1, x>, std::integral_constant<Integral2, y>> {
    using type = std::integral_constant<Integral2, y>;
};

template <int X>
using mi = std::integral_constant<int, X>;

TEST(foldr, adding) {
    EXPECT_EQ(10, (optimus::foldr1<add, mi<1>, mi<2>, mi<3>, mi<4>>::type::value));
    EXPECT_EQ(1, (optimus::foldr1<add, mi<1>>::type::value));
    EXPECT_EQ(2, (optimus::foldr1<add, mi<1>, mi<1>>::type::value));
}

TEST(foldr, first_argument) {
    EXPECT_EQ(10, (optimus::foldr<mi<10>, store_second, mi<11>, mi<12>>::type::value));
    EXPECT_EQ(10, (optimus::foldr<mi<10>, store_second>::type::value));
    EXPECT_EQ(10, (optimus::foldr<mi<10>, store_second, mi<12>>::type::value));

    EXPECT_EQ(12, (optimus::foldr1<store_second, mi<10>, mi<11>, mi<12>>::type::value));
    EXPECT_EQ(10, (optimus::foldr1<store_second, mi<10>>::type::value));
    EXPECT_EQ(12, (optimus::foldr1<store_second, mi<10>, mi<12>>::type::value));
}

TEST(foldl, adding) {
    EXPECT_EQ(10, (optimus::foldl1<add, mi<1>, mi<2>, mi<3>, mi<4>>::type::value));
    EXPECT_EQ(1, (optimus::foldl1<add, mi<1>>::type::value));
    EXPECT_EQ(2, (optimus::foldl1<add, mi<1>, mi<1>>::type::value));
}

TEST(foldl, first_argument) {
    EXPECT_EQ(10, (optimus::foldl<store_first, mi<10>, mi<11>, mi<12>>::type::value));
    EXPECT_EQ(10, (optimus::foldl<store_first, mi<10>>::type::value));
    EXPECT_EQ(10, (optimus::foldl<store_first, mi<10>, mi<12>>::type::value));

    EXPECT_EQ(10, (optimus::foldl1<store_first, mi<10>, mi<11>, mi<12>>::type::value));
    EXPECT_EQ(10, (optimus::foldl1<store_first, mi<10>>::type::value));
    EXPECT_EQ(10, (optimus::foldl1<store_first, mi<10>, mi<12>>::type::value));
}

#undef EXPECT_SAME_TYPE
#undef EXPECT_SAME_TYPE_AS
