#pragma once

template<
    typename T,
    size_t BLOCK_SIZE=256>
class FIFO_LockLess
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
    FIFO_LockLess(const FIFO_LockLess&) = delete;
    FIFO_LockLess(FIFO_LockLess&&) = delete;
    FIFO_LockLess& operator=(const FIFO_LockLess&) = delete;
    FIFO_LockLess& operator=(FIFO_LockLess&&) = delete;

    FIFO_LockLess()
        : blocks(nullptr)
        , first(nullptr)
        , last(nullptr)
    {
        auto block = NewBlock();
        blocks = block;
        first = &block->nodes[0];
        last = &block->nodes[0];
    }

    ~FIFO_LockLess()
    {
        Node* f = nullptr;
        Node* l = nullptr;
        f = first.exchange(f);
        l = last.exchange(l);
        assert(f != nullptr && l != nullptr);
        while (f!=l)
        {
            dataAlloc.destroy(&f->data);
            f = f->next;
        }
        Block* b = nullptr;
        b = blocks.exchange(b);
        assert(b != nullptr);
        while (b)
        {
            Block* t = b;
            b = t->next;
            blockAlloc.deallocate(t,1);
        }
    }

    void Push(const T& t)
    {
        Node* l = nullptr;
        l = last.exchange(l);
        while (!l)
        {
            std::this_thread::yield();
            l =  last.exchange(l);
        }
        dataAlloc.construct(&l->data, t);
        l = l->next;
        l = last.exchange(l);
        assert(!l);

        //Node* f = first;
        //while (!f)
        //{
        //    std::this_thread::yield();
        //    f = first;
        //}
        //if (f != l)
        //{
        //    // new_block contains a small ring buffer, add it to the big ring buffer
        //    Block* new_block = AddBlock();
        //    new_block->nodes[BLOCK_SIZE - 1].next = last->next;
        //    last->next = &new_block->nodes[0];
        //}
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
        bool res = false;
        Node* f = nullptr;
        f = first.exchange(f);
        while (!f)
        {
            std::this_thread::yield();
            f = first.exchange(f);
        }
        Node* l = last;
        while (!l)
        {
            std::this_thread::yield();
            l = last;
        }
        if (f!=l)
        {
            res = true;
            t = std::move(f->data);
            dataAlloc.destroy(&f->data);
            f = f->next;
        }
        f = first.exchange(f);
        assert(!f);
        return res;
    }

    std::vector<T> Pop(const size_t n = 1)
    {
        std::vector<T> res;
        res.reserve(n);
        T tmp;
        while ((res.size()<n) && TryPop(tmp))
        {
            res.emplace_back(std::move(tmp));
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
    Block * AddBlock()
    {
        Block* new_block = NewBlock();
        // store new_block in the linked list of blocks
        Block* block = nullptr;
        block = blocks.exchange(block);
        while (!block)
        {
            std::this_thread::yield();
            blocks.exchange(block);
        }
        block->next = new_block;
        block = blocks.exchange(block);
        assert(!block);
        return new_block;
    }

    DataAllocator dataAlloc;
    BlockAllocator blockAlloc;

    std::atomic<Block*> blocks;
    std::atomic<Node*> first;
    std::atomic<Node*> last;
};
