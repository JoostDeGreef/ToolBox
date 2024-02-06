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
};
template<>
class BinaryDataType<32>
{
public:
    using type = uint32_t;
};


template<size_t BITS>
class BinaryData : public BinaryDataOperations<typename BinaryDataType<BITS>::type>
{
public:
    using DataType = typename BinaryDataType<BITS>::type;

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


//template<size_t BITS>
//class BinaryData
//{
//protected:
//};
//
//template<>
//class BinaryData<16> : public BinaryDataOperations<uint16_t>
//{
//public:
//    using DataType = BinaryDataOperations<uint16_t>::DataType;
//
//protected:
//    using BinaryDataOperations<uint16_t>::data;
//
//public:
//    BinaryData()
//        : BinaryDataOperations()
//    {}
//    BinaryData(const DataType & d)
//        : BinaryDataOperations(d)
//    {}
//};
//
//template<>
//class BinaryData<32> : public BinaryDataOperations<uint32_t>
//{
//public:
//    using DataType = BinaryDataOperations<uint32_t>::DataType;
//
//protected:
//    using BinaryDataOperations<uint32_t>::data;
//
//public:
//    BinaryData()
//        : BinaryDataOperations()
//    {}
//    BinaryData(const DataType& d)
//        : BinaryDataOperations(d)
//    {}
//};

//template<>
//class BinaryData<64>
//{
//protected:
//    uint64_t data;
//};
//
//template<>
//class BinaryData<128>
//{
//protected:
//    uint64_t data[2];
//};
//
//template<>
//class BinaryData<256>
//{
//protected:
//    uint64_t data[4];
//};
