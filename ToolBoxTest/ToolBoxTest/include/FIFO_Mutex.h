#pragma once

template<
    typename T,
    size_t BLOCK_SIZE=256>
class FIFO_Mutex
{
    typedef T Data;
    typedef std::allocator<Data> DataAllocator;
    struct alignas(64) Node
    {
        Node(const Data & data)
            : data(data)
        {}
        Data data;
        Node* next;
    };
    struct Block
    {
        Node nodes[BLOCK_SIZE];
        Block* next;
    };
    typedef std::allocator<Block> BlockAllocator;
public:
    FIFO_Mutex(const FIFO_Mutex&) = delete;
    FIFO_Mutex(FIFO_Mutex&&) = delete;
    FIFO_Mutex& operator=(const FIFO_Mutex&) = delete;
    FIFO_Mutex& operator=(FIFO_Mutex&&) = delete;

    FIFO_Mutex()
        : blocks(nullptr)
        , first(nullptr)
        , last(nullptr)
        , free(0)
        , used(0)
    {
        auto block = NewBlock();
        free = BLOCK_SIZE;
        blocks = block;
        first = &block->nodes[0];
        last = &block->nodes[0];
    }

    ~FIFO_Mutex()
    {
        const std::lock_guard<std::mutex> lock(mtx);
        assert(first != nullptr && last != nullptr);
        while (first!=last)
        {
            std::allocator_traits<decltype(dataAlloc)>::destroy(dataAlloc, &first->data);
            first = first->next;
        }
        assert(blocks != nullptr);
        while (blocks)
        {
            Block* t = blocks;
            blocks = t->next;
            blockAlloc.deallocate(t,1);
        }
    }

    void Push(const T& t)
    {
        const std::lock_guard<std::mutex> lock(mtx);
        std::allocator_traits<decltype(dataAlloc)>::construct(dataAlloc, &last->data, t);
        if (free <= 1)
        {
            Block* new_block = AddBlock();
            new_block->nodes[BLOCK_SIZE - 1].next = last->next;
            last->next = &new_block->nodes[0];
            free += BLOCK_SIZE;
        }
        last = last->next;
        --free;
        ++used;
    }

    template <class ITER>
    void Push(ITER iter_first, ITER iter_last)
    {
        const std::lock_guard<std::mutex> lock(mtx);
        size_t n = std::distance(iter_first, iter_last);
        while (free <= n)
        {
            Block* new_block = AddBlock();
            new_block->nodes[BLOCK_SIZE - 1].next = last->next;
            last->next = &new_block->nodes[0];
            free += BLOCK_SIZE;
        }
        for (auto iter = iter_first; iter != iter_last; ++iter)
        {
            std::allocator_traits<decltype(dataAlloc)>::construct(dataAlloc, &last->data, *iter);
            last = last->next;
        }
        free -= n;
        used += n;
    }

    bool TryPop(T& t)
    {
        const std::lock_guard<std::mutex> lock(mtx);
        bool res = false;
        if (used>0)
        {
            res = true;
            t = std::move(first->data);
            std::allocator_traits<decltype(dataAlloc)>::destroy(dataAlloc, &first->data);
            first = first->next;
            --used;
            ++free;
        }
        return res;
    }

    std::vector<T> Pop(size_t n = 1)
    {
        const std::lock_guard<std::mutex> lock(mtx);
        std::vector<T> res;
        if (used < n) n = used;
        res.reserve(n);

        if (n > 0)
        {
            for (size_t i = 0; i < n; ++i)
            {
                res.emplace_back(std::move(first->data));
                std::allocator_traits<decltype(dataAlloc)>::destroy(dataAlloc, &first->data);
                first = first->next;
            }
            used -= n;
            free += n;
        }
        return res;
    }
private:
    // Create a new block of nodes, initialize as one ring;
    Block * NewBlock()
    {
        Block * block = blockAlloc.allocate(1);
        for (size_t i = 0; i < BLOCK_SIZE-1; ++i)
        {
            block->nodes[i].next = &block->nodes[i+1];
        }
        block->nodes[BLOCK_SIZE - 1].next = &block->nodes[0];
        block->next = nullptr;
        return block;
    }
    // store new_block in the linked list of blocks
    Block * AddBlock()
    {
        Block* new_block = NewBlock();
        new_block->next = blocks;
        blocks = new_block;
        return new_block;
    }

    DataAllocator dataAlloc;
    BlockAllocator blockAlloc;

    std::mutex mtx;

    Block* blocks;
    Node* first;
    Node* last;
    size_t free;
    size_t used;
};
