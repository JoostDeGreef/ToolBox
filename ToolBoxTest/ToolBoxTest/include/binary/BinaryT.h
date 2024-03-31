#pragma once

#include "BinaryData.h"

template<size_t EXPONENT,size_t MANTISSA>
class BinaryT : BinaryData<1 + EXPONENT + MANTISSA>
{
    using ThisType = BinaryT<EXPONENT, MANTISSA>;
    using DataType = typename BinaryData<1 + EXPONENT + MANTISSA>::DataType;
    using DataTypeSigned = typename BinaryData<1 + EXPONENT + MANTISSA>::DataTypeSigned;
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
    template<typename FP, std::enable_if_t<std::is_floating_point<FP>::value, bool> = true>
    explicit
    BinaryT(const FP& f)
    {
        this->operator=(f);
    }
    template<typename FP, std::enable_if_t<std::is_floating_point<FP>::value, bool> = true>
    BinaryT& operator = (const FP& f)
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
            data = GetData(f);
            break;
        case FP_SUBNORMAL:
            // TODO
            break;
        default:
            throw std::invalid_argument("Invalid floating point number");
        }
        return *this;
    }

private:
    void FlipSign() { data ^= MaskT<1, EXPONENT + MANTISSA>(); }
    static ThisType SetData(const DataType& input);
    static DataType GetData(const float& f);
    static DataType GetData(const double& d);
    // static DataType GetData(const long double& ld); // TODO?

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
    bool IsSubNormal() const {
        return ((data & MaskT<EXPONENT, MANTISSA>()) == 0); }
    bool IsNormal() const {
        return ((data & MaskT<EXPONENT, MANTISSA>()) > 0)
            && ((data & MaskT<EXPONENT, MANTISSA>()) < MaskT<EXPONENT, MANTISSA>()); }

    const static ThisType Zero;
    const static ThisType NaN;
    const static ThisType Infinite;

    bool GetSign() const
    {
        return IsNegative();
    }
    DataType GetMantissa() const
    {
        return data & MaskT<MANTISSA,0>();
    }
    DataTypeSigned GetExponent() const
    {
        return DataTypeSigned((data >> MANTISSA) & MaskT<EXPONENT, 0>()) - GetMaxExponent();
    }
    DataTypeSigned GetMaxExponent() const
    {
        return (1 << ((DataTypeSigned)EXPONENT-1)) - 1;
    }

    operator float() const
    {
        union
        {
            float f_data;    // 1,8,23
            uint32_t i_data;
        };
        i_data = GetSign() ? 0x80000000 : 0;
        auto mantissa = GetMantissa();
        auto exp = GetExponent();
        if (exp > GetMaxExponent())
        {
            if (mantissa != 0)
            {
                i_data |= 0x7FFFFFFF; // NAN
            }
            else
            {
                i_data |= 0x7F800000; // INF
            }
        }
        else if (exp > -GetMaxExponent())
        {
            if (exp <= -127)
            {
                // subnormal
                // TODO
            }
            else if (exp > 127)
            {
                i_data |= 0x7F800000; // too big to fit -> INF
            }
            else
            {
                i_data |= ((uint32_t)exp + 127) << 23;
                if (MANTISSA >= 23)
                {
                    i_data |= mantissa >> (int)(MANTISSA - 23);
                }
                else
                {
                    i_data |= mantissa << (23 - (int)MANTISSA);
                }
            }
        }
        else
        {
            // subnormal
            // TODO
        }
        return f_data;
    }
    operator double() const
    {
        // TODO
    }

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

template<size_t EXPONENT, size_t MANTISSA>
typename BinaryT<EXPONENT, MANTISSA>::DataType BinaryT<EXPONENT, MANTISSA>::GetData(const float& f)
{
    uint32_t& i_data = *(uint32_t*)&f;

    int32_t exp_i = ((i_data >> 23) & 0x000000FF) - 127;
    uint32_t mantissa_i = i_data & 0x007FFFFF;
    
    DataType sign = (i_data & 0x80000000) ? MaskT<1, EXPONENT + MANTISSA>() : DataType(0);
    DataType exp = 0;
    DataType mantissa = 0;

    auto ShiftMantissa = [](uint32_t m)
    {
        if (MANTISSA < 23)
        {
            return DataType(m >> (23 - (int)MANTISSA));
        }
        else
        {
            return DataType(m << ((int)MANTISSA - 23));
        }
    };

    if (exp_i == 128)
    {
        // infinity or NAN
        exp = MaskT<EXPONENT, MANTISSA>();
        mantissa = ShiftMantissa(mantissa_i);
    }
    else if (exp_i > 15) // TODO: make this depend on the output type
    {
        // overflow to infinity
        exp = MaskT<EXPONENT, MANTISSA>();
    }
    else if (exp_i > -15) // TODO: make this depend on the output type
    {
        // normalized case. 
        // TODO: proper rounding?
        mantissa = ShiftMantissa(mantissa_i);
        exp = DataType(exp_i + (1 << ((int)EXPONENT - 1)) - 1);
        exp = exp << (int)MANTISSA;
    }
    else if (exp_i > -25)
    {
        // convert to subnormal
        // TODO
    }
    else 
    {
        // zero, or underflow
    }
    return sign | exp | mantissa;
}
template<size_t EXPONENT, size_t MANTISSA>
typename BinaryT<EXPONENT, MANTISSA>::DataType BinaryT<EXPONENT, MANTISSA>::GetData(const double& d)
{
    // TODO
}
