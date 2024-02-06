#pragma once

template<typename DATA>
class BinaryDataOperations
{
public:
    using DataType = DATA;

protected:
    DataType data;

public:
    BinaryDataOperations()
        : data()
    {}
    explicit
    BinaryDataOperations(const DataType & d)
        : data(d)
    {}

    template<int BITS,int SHIFT>
    static constexpr DataType MaskT()
    {
        const int bits = BITS;
        const int shift = SHIFT;
        const int S = 8 * sizeof(DataType);
        const int R = S - bits;
        const DataType B = ~DataType(0);
        DataType mask = (B >> R) << SHIFT;
        return mask;
    }
    //template<int BITS, int SHIFT>
    //static constexpr DataType MaskT()
    //{
    //    const int S = 8 * sizeof(DataType);
    //    DataType mask = ((~DataType(0)) >> (S - BITS)) << SHIFT;
    //    return mask;
    //}
};

