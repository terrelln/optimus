#include <functional>
#include <limits>
#include <random>
#include <type_traits>

#include <gtest/gtest.h>

#include <optimus/functional.h>

template <typename T>
class unary_operations : public ::testing::Test {
  public:
};

template <typename T>
class binary_operations : public ::testing::Test {
  public:
};

template <typename T>
class AllOperations : public ::testing::Test {
  public:
};

#define FN_(Op) \
    std::pair<optimus::Op<int>, std::Op<int>>, \
    std::pair<optimus::Op<char>, std::Op<char>>, \
    std::pair<optimus::Op<bool>, std::Op<bool>>

typedef ::testing::Types<
    FN_(negate),
    FN_(logical_not)
> unary_types;

typedef ::testing::Types<
    FN_(plus),
    FN_(minus),
    FN_(multiplies),
    FN_(divides),
    FN_(modulus),
    FN_(equal_to),
    FN_(not_equal_to),
    FN_(greater),
    FN_(greater_equal),
    FN_(less_equal),
    FN_(logical_and),
    FN_(logical_or),
    FN_(bit_and),
    FN_(bit_or),
    FN_(bit_xor)
> binary_types;

#undef FN_

TYPED_TEST_CASE(unary_operations, unary_types);
TYPED_TEST_CASE(binary_operations, binary_types);

template <typename Op, typename T = int>
struct random_input {
    static T gen(T lower = std::numeric_limits<T>::min(), T upper = std::numeric_limits<T>::max()) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<T> dis(lower, upper);
        return dis(gen);
    }
};

template <typename T>
struct random_input<optimus::divides<T>, T> {
    static T gen(T lower = std::numeric_limits<T>::min(), T upper = std::numeric_limits<T>::max()) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<T> dis(lower, upper);
        auto r = dis(gen);
        return r == 0 ? 1 : r;
    }
};

template <typename T>
struct random_input<optimus::modulus<T>, T> {
    static T gen(T lower = std::numeric_limits<T>::min(), T upper = std::numeric_limits<T>::max()) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<T> dis(lower, upper);
        auto r = dis(gen);
        return r == 0 ? 1 : r;
    }
};

template <typename Op>
struct random_input<Op, bool> {
    static bool gen(bool lower = std::numeric_limits<bool>::min(), bool upper = std::numeric_limits<bool>::max()) {
        return random_input<Op, int>::gen(0, 1);
    }
};

template <>
struct random_input<optimus::divides<bool>, bool> {
    static bool gen(bool lower = std::numeric_limits<bool>::min(), bool upper = std::numeric_limits<bool>::max()) {
        return true;
    }
};

template <>
struct random_input<optimus::modulus<bool>, bool> {
    static bool gen(bool lower = std::numeric_limits<bool>::min(), bool upper = std::numeric_limits<bool>::max()) {
        return true;
    }
};

TYPED_TEST(unary_operations, same_as_std_on_random_inputs) {
    using optimus_op = typename TypeParam::first_type;
    using std_op     = typename TypeParam::second_type;
    using type       = typename std_op::result_type;

    optimus_op oop;
    std_op     sop;

    for (size_t i = 0; i < 5; ++i) {
        auto r1 = random_input<optimus_op, type>::gen();
        EXPECT_EQ(sop(r1), oop(r1));
    }
}

TYPED_TEST(binary_operations, same_as_std_on_random_inputs) {
    using optimus_op = typename TypeParam::first_type;
    using std_op     = typename TypeParam::second_type;
    using type       = typename std_op::result_type;

    optimus_op oop;
    std_op     sop;

    for (size_t i = 0; i < 1000; ++i) {
        auto r1a = random_input<optimus_op, type>::gen();
        auto r1b = random_input<optimus_op, type>::gen();

        EXPECT_EQ(sop(r1a, r1b), oop(r1a, r1b));
    }
}

TYPED_TEST(unary_operations, constexpr_compatible) {
    using optimus_op = typename TypeParam::first_type;
    using type       = typename optimus_op::result_type;

    constexpr optimus_op op;

    constexpr optimus_op copy(op);
    constexpr optimus_op move(optimus_op{});

    using f = std::integral_constant<int, op(0)>;
    EXPECT_EQ(optimus_op{}(0), f::value);
    using t = std::integral_constant<int, op(1)>;
    EXPECT_EQ(optimus_op{}(1), t::value);
}

TYPED_TEST(binary_operations, constexpr_compatible) {
    using optimus_op = typename TypeParam::first_type;
    using type       = typename optimus_op::result_type;

    constexpr optimus_op op;

    constexpr optimus_op copy(op);
    constexpr optimus_op move(optimus_op{});

    using c = std::integral_constant<int, op(0, 1)>;
    EXPECT_EQ(optimus_op{}(0, 1), c::value);
    using d = std::integral_constant<int, op(1, 1)>;
    EXPECT_EQ(optimus_op{}(1, 1), d::value);
}

TYPED_TEST(unary_operations, has_required_typedefs) {
    using optimus_op = typename TypeParam::first_type;
    using std_op = typename TypeParam::second_type;

    using result_type = typename optimus_op::result_type;
    using std_result_type = typename std_op::result_type;
    EXPECT_TRUE((std::is_same<std_result_type, result_type>::value));

    using argument_type = typename optimus_op::argument_type;
    using std_argument_type = typename std_op::argument_type;
    EXPECT_TRUE((std::is_same<std_argument_type, argument_type>::value));
}

TYPED_TEST(binary_operations, has_required_typedefs) {
    using optimus_op = typename TypeParam::first_type;
    using std_op = typename TypeParam::second_type;

    using result_type = typename optimus_op::result_type;
    using std_result_type = typename std_op::result_type;
    EXPECT_TRUE((std::is_same<std_result_type, result_type>::value));

    using first_argument_type = typename optimus_op::first_argument_type;
    using std_first_argument_type = typename std_op::first_argument_type;
    EXPECT_TRUE((std::is_same<std_first_argument_type, first_argument_type>::value));

    using second_argument_type = typename optimus_op::second_argument_type;
    using std_second_argument_type = typename std_op::first_argument_type;
    EXPECT_TRUE((std::is_same<std_second_argument_type, first_argument_type>::value));
}
