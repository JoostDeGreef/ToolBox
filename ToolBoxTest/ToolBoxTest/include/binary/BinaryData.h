#pragma once

#include "BinaryDataOperations.h"

template<size_t BITS>
class BinaryDataType
{
// Unsupported binary floating point format
};
template<>
class BinaryDataType<16>
{
public:
    using type = uint16_t;
    using type_signed = int16_t;
};
template<>
class BinaryDataType<32>
{
public:
    using type = uint32_t;
    using type_signed = int32_t;
};
template<>
class BinaryDataType<64>
{
public:
    using type = uint64_t;
    using type_signed = int64_t;
};


template<size_t BITS>
class BinaryData : public BinaryDataOperations<typename BinaryDataType<BITS>::type>
{
public:
    using DataType = typename BinaryDataType<BITS>::type;
    using DataTypeSigned = typename BinaryDataType<BITS>::type_signed;

protected:
    using BinaryDataOperations<DataType>::data;

public:
    BinaryData()
        : BinaryDataOperations()
    {}
    BinaryData(const DataType& d)
        : BinaryDataOperations(d)
    {}
};


