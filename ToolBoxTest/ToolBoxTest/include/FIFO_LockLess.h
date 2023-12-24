#pragma once

template<
    typename T,
    size_t BLOCK_SIZE=256>
class FIFO_LockLess
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
    FIFO_LockLess(const FIFO_LockLess&) = delete;
    FIFO_LockLess(FIFO_LockLess&&) = delete;
    FIFO_LockLess& operator=(const FIFO_LockLess&) = delete;
    FIFO_LockLess& operator=(FIFO_LockLess&&) = delete;

    FIFO_LockLess()
        : blocks(nullptr)
        , store(nullptr)
        , first(nullptr)
        , last(nullptr)
    {}

    ~FIFO_LockLess()
    {
        while (first)
        {
            auto t = first;
            first = t->next;
            alloc.destroy(t);
        }
        while (blocks)
        {
            Node* t = blocks;
            blocks = t->next;
            alloc.deallocate(blocks, BLOCK_SIZE);
        }
    }

    void Push(const T& t)
    {
        Node* p = GetRawNodeFromStore();
        alloc.construct(p, t);
        // TODO
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
        for (auto iter = iter_first; iter != iter_last; ++iter)
        {
            Push(*iter);
        }
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
            alloc.destroy(p);
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
            alloc.destroy(p);
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
    Node* GetRawNodeFromStore()
    {
    }

    Allocator alloc;
    Node* blocks;
    Node* store;
    Node* first;
    Node* last;
};
