#include <functional>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <tuple>

#include <gtest/gtest.h>

#include <optimus/utility.h>

#define EXPECT_SAME_TYPE(T, U) \
    EXPECT_TRUE((::std::is_same<T, U>::value))

#define EXPECT_SAME_TYPE_AS(T, Value) \
    EXPECT_SAME_TYPE(T, decltype(Value))

struct A { };
struct B { };
struct C { };
struct D { };

struct Fn {
    Fn() { }
    A operator()() const&;
    B operator()() &;
    C operator()() &&;
    D operator()() const &&;
};

struct f {
    A operator()(const int&);
    B operator()(int&);
    C operator()(int&&);
    D operator()(const int&&);
};
struct g{
    template <typename T, typename... Args>
    const A operator()(T&&, const char&, Args&&...);

    template <typename T, typename... Args>
    A operator()(T&&, char&, Args&&...);

    template <typename T, typename... Args>
    const A& operator()(T&&, bool, Args&&...);

    template <typename T, typename... Args>
    A&& operator()(T&&, int, Args&&...);
};

TEST(apply, function_forwarding) {
    const Fn cf;
    Fn mf;
    EXPECT_SAME_TYPE_AS(A, (optimus::apply(cf, std::forward_as_tuple())));
    EXPECT_SAME_TYPE_AS(B, (optimus::apply(mf, std::forward_as_tuple())));
    EXPECT_SAME_TYPE_AS(C, (optimus::apply(Fn{}, std::forward_as_tuple())));
    EXPECT_SAME_TYPE_AS(D, (optimus::apply(std::move(cf), std::forward_as_tuple())));
}

TEST(apply, argument_forwarding) {
    const int x = 0;
    int y;
    EXPECT_SAME_TYPE_AS(A, (optimus::apply(f{}, std::forward_as_tuple(x))));
    EXPECT_SAME_TYPE_AS(B, (optimus::apply(f{}, std::forward_as_tuple(y))));
    EXPECT_SAME_TYPE_AS(C, (optimus::apply(f{}, std::forward_as_tuple(std::move(y)))));
    EXPECT_SAME_TYPE_AS(D, (optimus::apply(f{}, std::forward_as_tuple(std::move(x)))));
}

TEST(apply, return_forwarding) {
    const char cc = 0;
    char c;
    bool b;
    int i;
    EXPECT_SAME_TYPE_AS(const A, (optimus::apply(g{},
        std::forward_as_tuple(0, cc, false, std::string{"hello world"}))));
    EXPECT_SAME_TYPE_AS(A, (optimus::apply(g{},
        std::forward_as_tuple(0, c, false, std::string{"hello world"}))));
    EXPECT_SAME_TYPE_AS(const A&, (optimus::apply(g{},
        std::forward_as_tuple(0, b, false, std::string{"hello world"}))));
    EXPECT_SAME_TYPE_AS(A&&, (optimus::apply(g{},
        std::forward_as_tuple(0, i, false, std::string{"hello world"}))));
}

bool free_function(int x, char c) {
    return x == c;
}

TEST(apply, free_function) {
    EXPECT_EQ(false, (optimus::apply(free_function, std::forward_as_tuple(5, 3))));
    EXPECT_EQ(true, (optimus::apply(free_function, std::forward_as_tuple(5, 5))));
}

TEST(apply, lambda) {
    EXPECT_EQ(5, (optimus::apply([](const int& x, std::string y) { return y.size(); },
            std::forward_as_tuple(3, "hello"))));
}

struct S {
    S() : b_(false) { }
    S(bool b) : b_(b) { }
    bool foo() { return b_; }
    bool b_;
};

TEST(apply, member_function) {
    S s(true);
    EXPECT_EQ(true, (optimus::apply(std::mem_fn(&S::foo), std::forward_as_tuple(&s))));
};

TEST(integer_sequence, basic) {
    EXPECT_EQ(5, (optimus::integer_sequence<int, 4, 3, 0, 99, 1>::size()));
    EXPECT_SAME_TYPE(int, optimus::integer_sequence<int>::value_type);
    EXPECT_SAME_TYPE(char, optimus::integer_sequence<char>::value_type);
    EXPECT_EQ(0, (optimus::integer_sequence<int>::size()));
    EXPECT_EQ(1, (optimus::integer_sequence<char, 4>{}.size()));
}

TEST(index_sequence, basic) {
    EXPECT_EQ(3, (optimus::index_sequence<0, 0, 1>::size()));
    EXPECT_SAME_TYPE(std::size_t, optimus::index_sequence<>::value_type);
}

#define TEST_INTEGER_SEQUENCE(End, ...) \
    EXPECT_TRUE((::std::is_same< \
        optimus::integer_sequence<char, ##__VA_ARGS__ >, \
        optimus::make_integer_sequence<char, End >>::value)); \
    EXPECT_TRUE((::std::is_same< \
        optimus::integer_sequence<int, ##__VA_ARGS__ >, \
        optimus::make_integer_sequence<int, End >>::value)); \
    EXPECT_TRUE((::std::is_same< \
        optimus::integer_sequence<std::size_t, ##__VA_ARGS__ >, \
        optimus::make_integer_sequence<std::size_t, End >>::value))



//    TEST_INTEGER_SEQUENCE_IMPL(char, End, EXPAND(__VA_ARGS__) ); \
    TEST_INTEGER_SEQUENCE_IMPL(int, End, EXPAND(__VA_ARGS__) ); \
    TEST_INTEGER_SEQUENCE_IMPL(std::size_t, End, EXPAND(__VA_ARGS__) )


TEST(make_integer_sequence, works) {
    TEST_INTEGER_SEQUENCE( 0);
    TEST_INTEGER_SEQUENCE( 1, 0);
    TEST_INTEGER_SEQUENCE( 2, 0, 1);
    TEST_INTEGER_SEQUENCE( 3, 0, 1, 2);
    TEST_INTEGER_SEQUENCE( 4, 0, 1, 2, 3);
    TEST_INTEGER_SEQUENCE( 5, 0, 1, 2, 3, 4);
    TEST_INTEGER_SEQUENCE( 6, 0, 1, 2, 3, 4, 5);
    TEST_INTEGER_SEQUENCE( 7, 0, 1, 2, 3, 4, 5, 6);
    TEST_INTEGER_SEQUENCE( 8, 0, 1, 2, 3, 4, 5, 6, 7);
    TEST_INTEGER_SEQUENCE( 9, 0, 1, 2, 3, 4, 5, 6, 7, 8);
    TEST_INTEGER_SEQUENCE(10, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    TEST_INTEGER_SEQUENCE(11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    TEST_INTEGER_SEQUENCE(12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    TEST_INTEGER_SEQUENCE(13, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    TEST_INTEGER_SEQUENCE(14, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
    TEST_INTEGER_SEQUENCE(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    TEST_INTEGER_SEQUENCE(16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    TEST_INTEGER_SEQUENCE(17, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    TEST_INTEGER_SEQUENCE(18, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
    TEST_INTEGER_SEQUENCE(19, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18);
    TEST_INTEGER_SEQUENCE(20, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19);
    TEST_INTEGER_SEQUENCE(21, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
}

#undef TEST_INTEGER_SEQUENCE
#undef TEST_INTEGER_SEQUENCE_IMPL

#define TEST_INDEX_SEQUENCE(End) \
    EXPECT_TRUE((::std::is_same< \
        optimus::make_integer_sequence<std::size_t, End>, \
        optimus::make_index_sequence<End>>::value))

TEST(make_index_sequence, works) {
    TEST_INDEX_SEQUENCE( 0);
    TEST_INDEX_SEQUENCE( 1);
    TEST_INDEX_SEQUENCE( 2);
    TEST_INDEX_SEQUENCE( 3);
    TEST_INDEX_SEQUENCE( 4);
    TEST_INDEX_SEQUENCE( 5);
    TEST_INDEX_SEQUENCE( 6);
    TEST_INDEX_SEQUENCE( 7);
    TEST_INDEX_SEQUENCE( 8);
    TEST_INDEX_SEQUENCE( 9);
    TEST_INDEX_SEQUENCE(10);
    TEST_INDEX_SEQUENCE(11);
    TEST_INDEX_SEQUENCE(12);
    TEST_INDEX_SEQUENCE(13);
    TEST_INDEX_SEQUENCE(14);
    TEST_INDEX_SEQUENCE(15);
    TEST_INDEX_SEQUENCE(16);
    TEST_INDEX_SEQUENCE(17);
    TEST_INDEX_SEQUENCE(18);
    TEST_INDEX_SEQUENCE(19);
    TEST_INDEX_SEQUENCE(20);
    TEST_INDEX_SEQUENCE(21);
}

#undef TEST_INDEX_SEQUENCE

TEST(index_sequence_for, works) {
    EXPECT_TRUE((::std::is_same<
        optimus::integer_sequence<std::size_t>,
        optimus::index_sequence_for<>>::value));
    EXPECT_TRUE((::std::is_same<
        optimus::integer_sequence<std::size_t, 0>,
        optimus::index_sequence_for<int>>::value));
    EXPECT_TRUE((::std::is_same<
        optimus::integer_sequence<std::size_t, 0, 1>,
        optimus::index_sequence_for<int, bool>>::value));
    EXPECT_TRUE((::std::is_same<
        optimus::integer_sequence<std::size_t, 0, 1, 2>,
        optimus::index_sequence_for<int, char, bool>>::value));
}

#undef EXPECT_SAME_TYPE
#undef EXPECT_SAME_TYPE_AS
