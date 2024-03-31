
#include "../include/ToolBox.h"
#include "../gtest/gtest.h"

using namespace testing;
using namespace ToolBox;

template<typename T>
class BinaryTest : public Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

using MyTypes = ::testing::Types<
    Binary16,
    Binary32,
    Binary64>;
TYPED_TEST_SUITE(BinaryTest, MyTypes);

TYPED_TEST(BinaryTest, Sign)
{
    TypeParam b;
    bool neg = b.IsNegative();
    bool pos = b.IsPositive();
    EXPECT_FALSE(neg);
    EXPECT_TRUE(pos);
}

TYPED_TEST(BinaryTest, Infinite)
{
    TypeParam b = TypeParam::Infinite;
    EXPECT_TRUE(b.IsInfinite());
    EXPECT_FALSE(b.IsNaN());
    EXPECT_FALSE(b.IsZero());
    EXPECT_TRUE(b.IsPositive());
    EXPECT_FALSE(b.IsNegative());
    EXPECT_TRUE(std::isinf((float)b));
    b = -b;
    EXPECT_TRUE(b.IsInfinite());
    EXPECT_FALSE(b.IsNaN());
    EXPECT_FALSE(b.IsZero());
    EXPECT_FALSE(b.IsPositive());
    EXPECT_TRUE(b.IsNegative());
    b = std::numeric_limits<float>::infinity();
    EXPECT_TRUE(std::isinf((float)b));
}

TYPED_TEST(BinaryTest, NaN)
{
    TypeParam b = TypeParam::NaN;
    EXPECT_FALSE(b.IsInfinite());
    EXPECT_TRUE(b.IsNaN());
    EXPECT_FALSE(b.IsZero());
    EXPECT_TRUE(std::isnan((float)b));
}

TYPED_TEST(BinaryTest, Zero)
{
    TypeParam b = TypeParam::Zero;
    EXPECT_FALSE(b.IsInfinite());
    EXPECT_FALSE(b.IsNaN());
    EXPECT_TRUE(b.IsZero());
    EXPECT_FLOAT_EQ(0.0f, (float)b);
}

TYPED_TEST(BinaryTest, ConversionFloat)
{
    TypeParam b(1.0f);
    EXPECT_FALSE(b.IsInfinite());
    EXPECT_FALSE(b.IsNaN());
    EXPECT_FALSE(b.IsZero());
    EXPECT_TRUE(b.IsPositive());
    EXPECT_FALSE(b.IsNegative());
    EXPECT_FLOAT_EQ(1.0f, (float)b);
}
