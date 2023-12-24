
#include "../include/ToolBox.h"
#include "../gtest/gtest.h"

using namespace testing;
using namespace ToolBox;

class ThreadPoolTest : public Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ThreadPoolTest, ForAll_none)
{
    ThreadPool pool(0);
    std::vector<std::string> v = {"abc", "bca", "acb"};
    auto res = pool.ForAll(v, [](const std::string& s)
        {
            auto t = s;
            std::sort(t.begin(), t.end());
            SUCCEED() << "Processed " << s << " into " << t;
            return t;
        });
    EXPECT_EQ(v[0], res[2]);
}

TEST_F(ThreadPoolTest, ForAll_few)
{
    ThreadPool pool(2);
    std::vector<std::string> v = { "abc", "bca", "acb" };
    auto res = pool.ForAll(v, [](const std::string& s)
        {
            auto t = s;
            std::sort(t.begin(), t.end());
            SUCCEED() << "Processed " << s << " into " << t;
            return t;
        });
    EXPECT_EQ(v[0], res[2]);
}

TEST_F(ThreadPoolTest, ForAll_many)
{
    ThreadPool pool(100);
    std::vector<std::string> v = { "abc", "bca", "acb" };
    auto res = pool.ForAll(v, [](const std::string& s)
        {
            auto t = s;
            std::sort(t.begin(), t.end());
            SUCCEED() << "Processed " << s << " into " << t;
            return t;
        });
    EXPECT_EQ(v[0], res[2]);
}

TEST_F(ThreadPoolTest, Enqueue_none)
{
    ThreadPool pool(0);
    std::string s = "bca";
    auto res = pool.Enqueue([](const std::string& s)
        {
            auto t = s;
            std::sort(t.begin(), t.end());
            SUCCEED() << "Processed " << s << " into " << t;
            return t;
        },s);
    EXPECT_EQ(std::string("abc"), res.get());
}

TEST_F(ThreadPoolTest, Enqueue_few)
{
    ThreadPool pool(2);
    std::string s = "bca";
    auto res = pool.Enqueue([](const std::string& s)
        {
            auto t = s;
            std::sort(t.begin(), t.end());
            SUCCEED() << "Processed " << s << " into " << t;
            return t;
        }, s);
    EXPECT_EQ(std::string("abc"), res.get());
}


