#include <functional>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <tuple>

#include <gtest/gtest.h>

#include <optimus/tuple.h>

TEST(tuple, compiles) {
    EXPECT_EQ(1, (optimus::get<2>(optimus::make_tuple(3, 2, 1))));
}
