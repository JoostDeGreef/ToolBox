
#include "../include/ToolBox.h"
#include "../gtest/gtest.h"

using namespace testing;
using namespace ToolBox;

template<typename T>
class FIFOInterfaceTest : public Test
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
TYPED_TEST_SUITE(FIFOInterfaceTest, MyTypes);

TYPED_TEST(FIFOInterfaceTest, PushTryPop)
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

TYPED_TEST(FIFOInterfaceTest, PushPop)
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

template<typename T>
class FIFOConcurrencyTest : public Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

public:
    template<typename FIFO>
    std::vector<std::vector<size_t >> RunConcurrentTest(size_t nr_consumers, size_t nr_producers, size_t nr_batch_consumers, size_t nr_batch_producers, size_t data_per_consumer)
    {
        FIFO q;
        std::vector<std::vector<size_t >> results;
        std::vector<std::thread> threads;
        auto thread_func_consumer = [&](std::vector<size_t>& r)
        {
            size_t j;
            for (size_t i = 0; i < data_per_consumer;)
            {
                if (q.TryPop(j))
                {
                    r.push_back(j);
                    ++i;
                }
            }
        };
        auto thread_func_producer = [&](size_t min,size_t max)
        {
            for (size_t i = min; i < max; ++i)
            {
                q.Push(i);
            }
        };
        auto thread_func_batch_consumer = [&](std::vector<size_t>& r)
        {
            while (r.size() < data_per_consumer)
            {
                std::vector<size_t> v = q.Pop(data_per_consumer - r.size());
                r.insert(r.end(), v.begin(), v.end());
            }
        };
        auto thread_func_batch_producer = [&](size_t min, size_t max)
        {
            std::vector<size_t> v;
            v.reserve(max - min);
            for (size_t i = min; i < max; ++i)
            {
                v.emplace_back(i);
            }
            q.Push(v.begin(),v.end());
        };
        results.resize(nr_consumers+nr_batch_consumers);
        for (size_t i = 0; i < nr_consumers; ++i)
        {
            threads.emplace_back(thread_func_consumer, std::ref(results[i]));
        }
        for (size_t i = 0; i < nr_batch_consumers; ++i)
        {
            threads.emplace_back(thread_func_batch_consumer, std::ref(results[i+ nr_consumers]));
        }
        size_t total = (nr_consumers+nr_batch_consumers) * data_per_consumer;
        size_t producers = nr_batch_producers + nr_producers;
        for (size_t i = 0; i < nr_producers; ++i)
        {
            size_t min = (total * i) / producers;
            size_t max = (total * (i+1)) / producers;
            if (i == producers - 1)
            {
                max = total;
            }
            threads.emplace_back(thread_func_producer, min, max);
        }
        for (size_t i = 0; i < nr_batch_producers; ++i)
        {
            size_t min = (total * (i + nr_producers)) / producers;
            size_t max = (total * (i + nr_producers + 1)) / producers;
            if (i == nr_batch_producers - 1)
            {
                max = total;
            }
            threads.emplace_back(thread_func_producer, min, max);
        }
        for (auto& thread : threads)
        {
            thread.join();
        }
        return results;
    }
};

using MyConcurrentTypes = ::testing::Types<
    FIFO_LockLess<size_t>,
    FIFO_LockLess<size_t, 2>>;
TYPED_TEST_SUITE(FIFOConcurrencyTest, MyConcurrentTypes);

TYPED_TEST(FIFOConcurrencyTest, PushTryPop_OneProducerFewConsumers)
{
    size_t data_per_consumer = 10000;
    auto res = RunConcurrentTest<TypeParam>(4, 1, 0, 0, data_per_consumer);
    // check results
    std::vector<size_t> result;
    for (auto & v: res)
    {
        EXPECT_EQ(data_per_consumer, v.size());
        EXPECT_TRUE(std::is_sorted(v.begin(), v.end()));
        result.insert(result.end(),v.begin(),v.end());
    }
    std::sort(result.begin(), result.end());
    for (size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(i, result[i]) << "Queued value not recovered";
    }
}

TYPED_TEST(FIFOConcurrencyTest, PushTryPop_FewProducersFewConsumers)
{
    size_t data_per_consumer = 10000;
    auto res = RunConcurrentTest<TypeParam>(4, 4, 0, 0, data_per_consumer);
    // check results
    std::vector<size_t> result;
    for (auto& v : res)
    {
        EXPECT_EQ(data_per_consumer, v.size());
        result.insert(result.end(), v.begin(), v.end());
    }
    std::sort(result.begin(), result.end());
    for (size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(i, result[i]) << "Queued value not recovered";
    }
}

TYPED_TEST(FIFOConcurrencyTest, PushTryPop_ManyProducersManyConsumers)
{
    size_t data_per_consumer = 10000;
    auto res = RunConcurrentTest<TypeParam>(32, 16, 0, 0, data_per_consumer);
    // check results
    std::vector<size_t> result;
    for (auto& v : res)
    {
        EXPECT_EQ(data_per_consumer, v.size());
        result.insert(result.end(), v.begin(), v.end());
    }
    std::sort(result.begin(), result.end());
    for (size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(i, result[i]) << "Queued value not recovered";
    }
}

TYPED_TEST(FIFOConcurrencyTest, PushTryPop_ManyProducersManyConsumers_BatchesAndSingles)
{
    size_t data_per_consumer = 10000;
    auto res = RunConcurrentTest<TypeParam>(32, 16, 16, 8, data_per_consumer);
    // check results
    std::vector<size_t> result;
    for (auto& v : res)
    {
        EXPECT_EQ(data_per_consumer, v.size());
        result.insert(result.end(), v.begin(), v.end());
    }
    std::sort(result.begin(), result.end());
    for (size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_EQ(i, result[i]) << "Queued value not recovered";
    }
}
