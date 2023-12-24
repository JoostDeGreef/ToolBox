
#include "../include/ToolBox.h"
#include "../gtest/gtest.h"

using namespace testing;
using namespace ToolBox;

class GenericTest : public Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(GenericTest, Noop)
{
}

