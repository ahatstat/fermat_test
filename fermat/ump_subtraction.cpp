//this file is included in ump.cpp
//64 bit subtract with borrow.  use compiler intrinsics if available. 
#if defined(_M_X64) && (defined(_W32) || defined(_W64))    // MSVC Windows X64
#include <intrin.h>
#define sub_borrow_u64 _subborrow_u64
#elif defined(__GNUC__) && defined(__x86_64__)  //GCC X64
#include <x86intrin.h>
#include "ump.hpp"
#define sub_borrow_u64 _subborrow_u64
#elif defined(__SIZEOF_INT128__)  //The compiler supports 128 bit arithmetic
static inline unsigned char sub_borrow_u64(unsigned char carry_in, uint64_t a, uint64_t b, uint64_t* sum)
{
    unsigned __int128 x = static_cast<unsigned __int128>(a) - static_cast<unsigned __int128>(b) - carry_in;
    unsigned char carry_out = (x >> 64) > 0 ? 1 : 0;
    *sum = static_cast<uint64_t>(x);
    return carry_out;
}
#else  //fallback
static inline unsigned char sub_borrow_u64(unsigned char carry_in, uint64_t a, uint64_t b, uint64_t* sum)
{
    //subtraction implemented as two's complement addition
    bool propagate = false;
    bool generate = false;
    unsigned char carry_out = 1;
    uint64_t x = 0;
    x = a + ~b;
    *sum = x + carry_in;
    propagate = x == -1;
    generate = x < a;
    carry_out = generate || (propagate && carry) ? 1 : 0;
    return carry_out;
}
#endif 

//32 bit subtract with borrow
static inline unsigned char sub_borrow_u32(unsigned char carry_in, uint32_t a, uint32_t b, uint32_t* sum)
{
    uint64_t x = static_cast<uint64_t>(a) - static_cast<uint64_t>(b) - carry_in;
    unsigned char carry_out = (x >> 32) > 0 ? 1 : 0;
    *sum = (uint32_t)x;
    return carry_out;
}

namespace ump {

    static inline unsigned char sub_borrow(unsigned char carry_in, uint32_t a, uint32_t b, uint32_t* sum)
    {
        return sub_borrow_u32(carry_in, a, b, sum);
    }

    static inline unsigned char sub_borrow(unsigned char carry_in, uint64_t a, uint64_t b, uint64_t* sum)
    {
        return sub_borrow_u64(carry_in, a, b, (unsigned long long*)sum);
    }
       
    template<int BITS>
    Ump<BITS> Ump<BITS>::sub(const Ump<BITS>& b) const
    {
        Ump<BITS> result;
        unsigned char carry = 0;
        for (auto i = 0; i < LIMBS; i++)
        {
            carry = sub_borrow(carry, m_limbs[i], b.m_limbs[i], &result.m_limbs[i]);
        }

        return result;
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::sub(int b) const
    {
        if (b < 0)
        {
            return add(static_cast<uint32_t>(-b));
        }
        return sub(static_cast<uint32_t>(b));
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::sub(uint32_t b) const
    {
        Ump result;
        result.m_limbs[0] = m_limbs[0] - b;
        uint8_t carry = result.m_limbs[0] > m_limbs[0] ? 1 : 0;
        for (int i = 1; i < LIMBS; i++)
        {
            result.m_limbs[i] = m_limbs[i] - carry;
            carry = result.m_limbs[i] > m_limbs[i] ? 1 : 0;
        }
        return result;
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::sub(uint64_t b) const
    {
        Ump<BITS> r = b;
        return sub(r);
    }

    //same as subtract, but results are stored in the current object
    template<int BITS>
    void Ump<BITS>::decrement(const Ump<BITS>& b)
    {
        unsigned char carry = 0;
        for (auto i = 0; i < LIMBS; i++)
        {
            carry = sub_borrow(carry, m_limbs[i], b.m_limbs[i], &m_limbs[i]);
        }
    }

    template<int BITS>
    void Ump<BITS>::decrement(int b)
    {
        if (b < 0)
            increment(static_cast<uint32_t>(-b));
        else
            decrement(static_cast<uint32_t>(b));
    }

    template<int BITS>
    void Ump<BITS>::decrement(uint32_t b)
    {
        limb_t temp = m_limbs[0];
        m_limbs[0] -= b;
        uint8_t carry = m_limbs[0] > temp ? 1 : 0;
        for (int i = 1; i < LIMBS; i++)
        {
            temp = m_limbs[i];
            m_limbs[i] -= carry;
            carry = m_limbs[i] > temp ? 1 : 0;
        }
    }
    template<int BITS>
    void Ump<BITS>::decrement(uint64_t b)
    {
        Ump<BITS> r = b;
        decrement(r);
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::negate()
    {
        Ump<BITS> zero;
        return zero.sub(*this);
    }

    template<int BITS>
    void Ump<BITS>::operator-=(const Ump<BITS>& b)
    {
        decrement(b);
    }

    template<int BITS>
    void Ump<BITS>::operator-=(int b)
    {
        decrement(b);
    }

    template<int BITS>
    void Ump<BITS>::operator-=(uint32_t b)
    {
        decrement(b);
    }

    template<int BITS>
    void Ump<BITS>::operator-=(uint64_t b)
    {
        Ump<BITS> r = b;
        decrement(r);
    }

    template<int BITS>
    Ump<BITS> Ump<BITS>::operator-()
    {
        return negate();
    }

    template<int BITS>
    Ump<BITS> operator - (const Ump<BITS>& lhs, const Ump<BITS>& rhs)
    {
        return lhs.sub(rhs);
    }

    template<int BITS>
    Ump<BITS> operator - (const Ump<BITS>& lhs, int rhs)
    {
        return lhs.sub(rhs);
    }

    template<int BITS>
    Ump<BITS> operator-(const Ump<BITS>& lhs, uint32_t rhs)
    {
        return lhs.sub(rhs);
    }

    template<int BITS>
    Ump<BITS> operator-(const Ump<BITS>& lhs, uint64_t rhs)
    {
        Ump<BITS> r = rhs;
        return lhs.sub(r);
    }

}
