#include <gtest/gtest.h>
#include "others/utils.hpp"

TEST(UtilsTest, InWorldRange_BasicTest)
{
    EXPECT_TRUE(in_world_range(0, 0, 100, 100));
    EXPECT_TRUE(in_world_range(50, 50, 100, 100));
    EXPECT_FALSE(in_world_range(-1, 0, 100, 100));
    EXPECT_FALSE(in_world_range(100, 0, 100, 100));
}

TEST(ColorTest, ColorConversion)
{
    Color red(255, 0, 0, 1.0f);

    EXPECT_FLOAT_EQ(red.r, 1.0f);
    EXPECT_FLOAT_EQ(red.g, 0.0f);
    EXPECT_FLOAT_EQ(red.b, 0.0f);
    EXPECT_FLOAT_EQ(red.a, 1.0f);
}

TEST(RandomTest, GetFloatReturnsValueInRange)
{
    // float value = Random_Machine::get_float();

    // EXPECT_GE(value, 0.0f) << "Float value should be >= 0";
    // EXPECT_LE(value, 1.0f) << "Float value should be <= 1";
}