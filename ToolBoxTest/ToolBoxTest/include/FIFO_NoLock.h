#pragma once

template<
    typename T,
    size_t BLOCK_SIZE=256>
class FIFO_NoLock
{
    typedef T Data;
    struct Node
    {
        Node(const Data & data)
            : data(data)
            , next(nullptr)
        {}
        Data data;
        Node* next;
    };
    typedef std::allocator<Node> Allocator;
public:
    FIFO_NoLock(const FIFO_NoLock&) = delete;
    FIFO_NoLock(FIFO_NoLock&&) = delete;
    FIFO_NoLock& operator=(const FIFO_NoLock&) = delete;
    FIFO_NoLock& operator=(FIFO_NoLock&&) = delete;

    FIFO_NoLock(const size_t reserve = 0)
        : first(nullptr)
        , last(nullptr)
    {
        FillStore(reserve);
    }

    ~FIFO_NoLock()
    {
        while (first)
        {
            auto t = first;
            first = t->next;
            std::allocator_traits<decltype(alloc)>::destroy(alloc, t);
        }
        for (Node* block : blocks)
        {
            std::allocator_traits<decltype(alloc)>::deallocate(alloc, block, BLOCK_SIZE);
        }
    }

    void Push(const T& t)
    {
        FillStore(1);
        Node* p = store.back();
        store.pop_back();
        std::allocator_traits<decltype(alloc)>::construct(alloc, p, t);
        if (last)
        {
            last->next = p;
            last = p;
        }
        else
        {
            first = p;
            last = p;
        }
    }

    template <class ITER>
    void Push(ITER iter_first, ITER iter_last)
    {
        typename std::iterator_traits<ITER>::difference_type n = std::distance(iter_first, iter_last);
        if (n > 0)
        {
            FillStore(n);
            size_t index = store.size() - 1 - n;
            for (auto iter = iter_first; iter != iter_last; ++iter)
            {
                Node* p = store[index++];
                std::allocator_traits<decltype(alloc)>::construct(alloc, p, *iter);
                if (last)
                {
                    last->next = p;
                }
                else
                {
                    first = p;
                }
                last = p;
            }
            store.resize(store.size() - n);
        }
    }

    bool IsEmpty()
    {
        return !first;
    }

    bool TryPop(T& t)
    {
        if (first == nullptr)
        {
            return false;
        }
        else
        {
            t = std::move(first->data);
            Node* p = first;
            first = p->next;
            std::allocator_traits<decltype(alloc)>::destroy(alloc, p);
            store.push_back(p);
            if (!first)
            {
                last = nullptr;
            }
            return true;
        }
    }

    std::vector<T> Pop(const size_t n = 1)
    {
        std::vector<T> res;
        res.reserve(n);
        while (first && (res.size()<n))
        {
            res.emplace_back(std::move(first->data));
            Node* p = first;
            first = first->next;
            std::allocator_traits<decltype(alloc)>::destroy(alloc, p);
            store.push_back(p);
        }
        if (!first)
        {
            last = nullptr;
        }
        return res;
    }

private:
    void AddBlock()
    {
        Node * block = alloc.allocate(BLOCK_SIZE);
        for (size_t i = 0; i < BLOCK_SIZE; ++i)
        {
            store.emplace_back(&block[i]);
        }
    }
    void FillStore(const size_t n)
    {
        while (store.size() < n)
        {
            AddBlock();
        }
    }

    Allocator alloc;
    std::vector<Node*> blocks;
    std::vector<Node*> store;
    Node* first;
    Node* last;
};
