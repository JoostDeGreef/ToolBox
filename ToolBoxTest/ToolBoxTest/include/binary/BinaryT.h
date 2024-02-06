#pragma once

#include "BinaryData.h"

template<size_t EXPONENT,size_t MANTISSA>
class BinaryT : BinaryData<1 + EXPONENT + MANTISSA>
{
    using ThisType = BinaryT<EXPONENT, MANTISSA>;
    using DataType = typename BinaryData<1 + EXPONENT + MANTISSA>::DataType;
    using BinaryData<1 + EXPONENT + MANTISSA>::data;

    constexpr static size_t bits_sign = 1;
    constexpr static size_t bits_exponent = EXPONENT;
    constexpr static size_t bits_mantissa = MANTISSA;

public:
    BinaryT()
        : BinaryData()
    {}
    BinaryT(const ThisType &other)
        : BinaryData(other.data)
    {}
    // move to c++20 and use concepts?
    template<typename FP, std::enable_if_t<std::is_floating_point<FP>::value, bool> = true>
    explicit
    BinaryT(const FP& f)
    {
        switch (std::fpclassify(f))
        {
        case FP_INFINITE:
            data = Infinite.data;
            break;
        case FP_NAN:
            data = NaN.data;
            break;
        case FP_ZERO:
            data = Zero.data;
            break;
        case FP_NORMAL:
            // TODO
            break;
        case FP_SUBNORMAL:
            // TODO
            break;
        default:
            throw std::invalid_argument("Invalid floating point number");
        }
    }

private:
    static ThisType SetData(const DataType& input);
    void FlipSign() { data ^= MaskT<1, EXPONENT + MANTISSA>(); }

public:
    bool IsZero() const {
        return (data & MaskT<EXPONENT + MANTISSA, 0>()) == 0; }
    bool IsNegative() const {
        return (data & MaskT<1, EXPONENT + MANTISSA>()) != 0; }
    bool IsPositive() const {
        return (data & MaskT<1, EXPONENT + MANTISSA>()) == 0; }
    bool IsNaN() const {
        return ((data & MaskT<EXPONENT, MANTISSA>()) == MaskT<EXPONENT, MANTISSA>())
            && ((data & MaskT<MANTISSA, 0>()) != 0); }
    bool IsInfinite() const {
        return ((data & MaskT<EXPONENT, MANTISSA>()) == MaskT<EXPONENT, MANTISSA>())
            && ((data & MaskT<MANTISSA, 0>()) == 0); }

    const static ThisType Zero;
    const static ThisType NaN;
    const static ThisType Infinite;

    ThisType operator - () const
    {
        ThisType temp = *this;
        temp.FlipSign();
        return temp;
    }
    ThisType operator + (const ThisType & other) const
    {

    }
};

template<size_t EXPONENT, size_t MANTISSA>
BinaryT<EXPONENT,MANTISSA> BinaryT<EXPONENT, MANTISSA>::SetData(const DataType& input)
{
    ThisType res;
    res.data = input;
    return res;
}

template<size_t EXPONENT, size_t MANTISSA>
const BinaryT<EXPONENT, MANTISSA> BinaryT<EXPONENT, MANTISSA>::Zero = BinaryT<EXPONENT, MANTISSA>::SetData(0);
template<size_t EXPONENT, size_t MANTISSA>
const BinaryT<EXPONENT, MANTISSA> BinaryT<EXPONENT, MANTISSA>::NaN = BinaryT<EXPONENT, MANTISSA>::SetData(MaskT<EXPONENT + MANTISSA, 0>());
template<size_t EXPONENT, size_t MANTISSA>
const BinaryT<EXPONENT, MANTISSA> BinaryT<EXPONENT, MANTISSA>::Infinite = BinaryT<EXPONENT, MANTISSA>::SetData(MaskT<EXPONENT, MANTISSA>());

