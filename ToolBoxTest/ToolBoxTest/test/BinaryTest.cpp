
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
    Binary32>;
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
    b = -b;
    EXPECT_TRUE(b.IsInfinite());
    EXPECT_FALSE(b.IsNaN());
    EXPECT_FALSE(b.IsZero());
    EXPECT_FALSE(b.IsPositive());
    EXPECT_TRUE(b.IsNegative());
}

TYPED_TEST(BinaryTest, NaN)
{
    TypeParam b = TypeParam::NaN;
    EXPECT_FALSE(b.IsInfinite());
    EXPECT_TRUE(b.IsNaN());
    EXPECT_FALSE(b.IsZero());
}

TYPED_TEST(BinaryTest, Zero)
{
    TypeParam b = TypeParam::Zero;
    EXPECT_FALSE(b.IsInfinite());
    EXPECT_FALSE(b.IsNaN());
    EXPECT_TRUE(b.IsZero());
}
