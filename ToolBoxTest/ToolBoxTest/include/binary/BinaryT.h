#pragma once

#include "BinaryData.h"

namespace TypeSelect
{
    template<bool, typename T1, typename T2>
    struct Pick
    {};
    template<typename T1, typename T2>
    struct Pick<true, T1, T2>
    {
        typedef T1 type;
    };
    template<typename T1, typename T2>
    struct Pick<false, T1, T2>
    {
        typedef T2 type;
    };

    template<typename T1, typename T2>
    struct Largest
    {
        typedef typename Pick<(sizeof(T1) > sizeof(T2)), T1, T2>::type type;
    };
}

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

    template<typename F>
    static DataType GetData(const F& f);

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

private:
    template<typename F>
    F Convert() const;
public:
    operator float() const
    {
        return Convert<float>();
    }
    operator double() const
    {
        return Convert<double>();
    }

    ThisType operator - () const
    {
        ThisType temp = *this;
        temp.FlipSign();
        return temp;
    }
    ThisType operator + () const
    {
        return *this;
    }
    ThisType operator - (const ThisType & other) const
    {

    }
    ThisType operator + (const ThisType& other) const
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
template<typename F>
typename BinaryT<EXPONENT, MANTISSA>::DataType BinaryT<EXPONENT, MANTISSA>::GetData(const F& f)
{
    using U = BinaryDataType<sizeof(F) * 8>::type;
    using I = BinaryDataType<sizeof(F) * 8>::type_signed;

    constexpr size_t MANTISSA_BITS = std::numeric_limits<F>::digits - 1; // bits in mantissa F
    constexpr size_t EXP_BITS = sizeof(F)*8 - MANTISSA_BITS - 1;         // bits in exponent F
    constexpr int MAX_EXP = (1 << (EXPONENT-1))-1;                       // max exp value for BinaryT

    U& i_data = *(U*)&f;

    I exp_i = ((i_data & (~(U)0 >> 1)) >> MANTISSA_BITS) - (1 << (EXP_BITS -1)) + 1;
    U mantissa_i = i_data & (~(U)0 >> (1+EXP_BITS));
    
    DataType sign = (i_data & ((U)1 << EXP_BITS+MANTISSA_BITS)) ? MaskT<1, EXPONENT + MANTISSA>() : DataType(0);
    DataType exp = 0;
    DataType mantissa = 0;

    auto ShiftMantissa = [&MANTISSA_BITS](U m)
    {
        if (MANTISSA < MANTISSA_BITS)
        {
            return DataType(m >> (MANTISSA_BITS - MANTISSA));
        }
        else
        {
            return DataType(DataType(m) << DataType(MANTISSA - MANTISSA_BITS));
        }
    };

    if (exp_i == (1 << (EXP_BITS - 1)))
    {
        // infinity or NAN
        exp = MaskT<EXPONENT, MANTISSA>();
        mantissa = ShiftMantissa(mantissa_i);
    }
    else if (exp_i > MAX_EXP)
    {
        // overflow to infinity
        exp = MaskT<EXPONENT, MANTISSA>();
    }
    else if (exp_i > -MAX_EXP)
    {
        // normalized case. 
        // TODO: proper rounding?
        mantissa = ShiftMantissa(mantissa_i);
        exp = DataType(exp_i + (1 << ((int)EXPONENT - 1)) - 1);
        exp = exp << (int)MANTISSA;
    }
    else
    {
        // convert to subnormal, or zero, or underflow
        // TODO
    }
    return sign | exp | mantissa;
}


template<size_t EXPONENT, size_t MANTISSA>
template<typename F>
F BinaryT<EXPONENT, MANTISSA>::Convert() const
{
    constexpr size_t MANTISSA_BITS = std::numeric_limits<F>::digits - 1; // bits in mantissa F
    constexpr size_t EXP_BITS = sizeof(F) * 8 - MANTISSA_BITS - 1;       // bits in exponent F

    using I = BinaryDataType<1 + EXP_BITS + MANTISSA_BITS>::type;
    union
    {
        F f_data;
        I i_data;
    };
    i_data = GetSign() ? ((I)1 << (I)(EXP_BITS + MANTISSA_BITS)) : (I)0;
    auto mantissa = GetMantissa();
    auto exp = GetExponent();
    if (exp > GetMaxExponent())
    {
        if (mantissa != 0)
        {
            i_data |= (~(I)0) >> 1; // NAN
        }
        else
        {
            constexpr auto f = std::numeric_limits<F>::infinity();
            i_data |= *(I*)&f; // INF
        }
    }
    else if (exp > -GetMaxExponent())
    {
        auto MAX_EXP = ((1 << EXP_BITS - 1) - 1);
        if (exp <= -MAX_EXP)
        {
            // subnormal
            // TODO
        }
        else if (exp > MAX_EXP)
        {
            constexpr auto f = std::numeric_limits<F>::infinity();
            i_data |= *(I*)&f; // INF
        }
        else
        {
            i_data |= (I)(exp + MAX_EXP) << (I)MANTISSA_BITS;
            if (MANTISSA == MANTISSA_BITS)
            {
                i_data |= mantissa;
            }
            else if (MANTISSA > MANTISSA_BITS)
            {
                I i = (I)(MANTISSA - MANTISSA_BITS);
                i_data |= mantissa >> i;
            }
            else
            {
                I i = (I)(MANTISSA_BITS - MANTISSA);
                i_data |= mantissa << i;
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
