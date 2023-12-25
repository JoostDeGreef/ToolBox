
#include "../include/ToolBox.h"
#include "../gtest/gtest.h"

using namespace testing;
using namespace ToolBox;

template<typename T>
class FIFOTest : public Test
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
    FIFO_NoLock<int>,
    FIFO_NoLock<int,2>,
    FIFO_LockLess<int>,
    FIFO_LockLess<int,2>> ;
TYPED_TEST_SUITE(FIFOTest, MyTypes);

TYPED_TEST(FIFOTest, PushTryPop)
{
    TypeParam q;
    std::vector<int> v{ 1,2,3 },r;
    int tmp;
    EXPECT_FALSE(q.TryPop(tmp));
    q.Push(v.begin(), v.end());
    while (q.TryPop(tmp))
    {
        r.emplace_back(tmp);
    }
    EXPECT_EQ(v, r);
}

TYPED_TEST(FIFOTest, PushPop)
{
    TypeParam q;
    std::vector<int> v{ 1,2,3 }, r;
    for (const int i : v)
    {
        q.Push(i);
    }
    r = q.Pop(4);
    EXPECT_EQ(v, r);
}

