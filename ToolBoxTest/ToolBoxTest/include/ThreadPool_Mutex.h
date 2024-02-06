#pragma once

class ThreadPool_Mutex
{
public:
    ThreadPool_Mutex(const ThreadPool_Mutex&) = delete;
    ThreadPool_Mutex(ThreadPool_Mutex&&) = delete;
    ThreadPool_Mutex& operator=(const ThreadPool_Mutex&) = delete;
    ThreadPool_Mutex& operator=(ThreadPool_Mutex&&) = delete;

    ThreadPool_Mutex(const int n = 0)
        : thread_count(0)
    {
        SetThreads(n);
    }

    ~ThreadPool_Mutex()
    {
        SetThreads(0);
    }

    template<typename Func, typename... Args>
    auto Enqueue(Func f, Args &... args)
    {
        using return_type = std::invoke_result_t<Func,Args &...>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(f, args...));
        std::future<return_type> res = task->get_future();
        if(thread_count>0)
        {
            std::unique_lock<std::mutex> lock(mtx);
            tasks.emplace([task]() -> bool { (*task)(); return true; });
            cv.notify_one();
        }
        else
        {
            (*task)();
        }
        return res;
    }

    template<typename TInput, typename Func, typename... CommonArgs>
    auto ForAll(const std::vector<TInput>& inputs, Func f, CommonArgs &... args)
    {
        using return_type = std::invoke_result_t<Func,TInput,CommonArgs &...>;

        std::vector<return_type> results;
        results.reserve(inputs.size());

        if (thread_count > 0)
        {
            std::vector<std::future<return_type>> futures;
            futures.reserve(inputs.size());
            {
                std::unique_lock<std::mutex> lock(mtx);
                for (const auto& input : inputs)
                {
                    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(f, input, args...));
                    std::future<return_type> res = task->get_future();
                    tasks.emplace([task]() -> bool { (*task)(); return true; });
                    futures.emplace_back(std::move(res));
                }
            }
            if (inputs.size() >= thread_count)
            {
                cv.notify_all();
            }
            else
            {
                for (int i = 0; i < thread_count; ++i)
                {
                    cv.notify_one();
                }
            }
            for (auto& f : futures)
            {
                results.emplace_back(std::move(f.get()));
            }
        }
        else
        {
            for (const auto& input : inputs)
            {
                results.emplace_back(f(input, args...));
            }
        }

        return results;
    }

    int GetNumberOfThreads() const 
    { 
        return thread_count; 
    }
    int SetNumberOfThreads(const int n = 0)
    {
        SetThreads(n);
        return thread_count;
    }
private:
    void Worker()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (tasks.empty())
            {
                cv.wait(lock, [&]() { return lock.owns_lock(); });
            }
            if (!tasks.empty())
            {
                auto task = std::move(tasks.front());
                tasks.pop();
                lock.unlock();
                if (!task())
                {
                    return;
                }
            }
        }
    }

    void SetThreads(int n)
    {
        using namespace std::chrono_literals;

        if (n < 0)
        {
            n = std::max(std::thread::hardware_concurrency(), (unsigned int)1);
        }

        if (n > thread_count)
        {
            for (int i = thread_count; i < n; ++i)
            {
                std::thread(&ThreadPool_Mutex::Worker, this).detach();
            }
        }
        else if (n < thread_count)
        {
            std::vector<std::future<void>> futures;
            futures.reserve(((size_t)thread_count)-((size_t)n));
            {
                std::unique_lock<std::mutex> lock(mtx);
                for (int i = n; i < thread_count; ++i)
                {
                    auto task = std::make_shared<std::packaged_task<void()>>([]() {});
                    std::future<void> res = task->get_future();
                    {
                        tasks.emplace([task]() -> bool { (*task)(); return false; });
                    }
                    futures.emplace_back(std::move(res));
                }
            }
            if (!n)
            {
                cv.notify_all();
            }
            else
            {
                for (size_t i = 0; i < futures.size(); ++i)
                {
                    cv.notify_one();
                }
            }
            for (auto& f : futures)
            {
                f.wait();
            }
        }
        thread_count = n;
    }

private:
    std::queue<std::function<bool()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    int thread_count;
};
