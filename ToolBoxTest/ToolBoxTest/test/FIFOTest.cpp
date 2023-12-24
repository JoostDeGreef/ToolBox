
#include "../include/ToolBox.h"
#include "../gtest/gtest.h"

using namespace testing;
using namespace ToolBox;

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

TEST_F(FIFOTest, PushTryPop)
{
    FIFO<int> q;
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

TEST_F(FIFOTest, PushPop)
{
    FIFO<int,2> q;
    std::vector<int> v{ 1,2,3 }, r;
    for (const int i : v)
    {
        q.Push(i);
    }
    r = q.Pop(4);
    EXPECT_EQ(v, r);
}

